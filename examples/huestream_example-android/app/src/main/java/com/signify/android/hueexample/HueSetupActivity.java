/*******************************************************************************
 Copyright (C) 2021 Signify Holding B.V.
 All Rights Reserved.
 *******************************************************************************/

package com.signify.android.hueexample;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.Spinner;
import android.widget.TextView;

import com.lighting.huestream.Bridge;
import com.lighting.huestream.FeedbackMessage;
import com.lighting.huestream.Group;
import com.lighting.huestream.GroupVector;

import java.util.ArrayList;
import java.util.List;


public class HueSetupActivity extends Activity implements HueHandler.HueFeedbackListener {

    private HueHandler hue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hue_setup);

        hue = ((HueExampleApplication) getApplication()).getHue();

        updateUI();

        hue.registerFeedbackListener(this);
    }

    public void onBackClicked(View view) {
        finish();
    }

    public void onConnectClicked(View view) {
        Intent intent = new Intent(this, HueConnectActivity.class);
        startActivity(intent);
    }

    public void onResetClicked(View view) {
        hue.reset();
    }

    public void onSetLightToGreen(View view) { hue.setLightsToGreen(); }

    public void onPlayVariousEffects(View view) { hue.showAnimatedLightsExample(); }

    @Override
    public void onFeedbackMessage(FeedbackMessage message) {
        if (message.GetId() == FeedbackMessage.Id.ID_USERPROCEDURE_FINISHED
                || message.GetRequestType() == FeedbackMessage.RequestType.REQUEST_TYPE_INTERNAL) {
            HueSetupActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    updateUI();
                }
            });
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        hue.deregisterFeedbackListener(this);
    }

    @Override
    public void onStop() {
        super.onStop();
        hue.stopStreaming();
    }

    private void updateUI() {
        setTextAndButtons();
        setGroups();
    }

    private void setTextAndButtons() {
        TextView txt = findViewById(R.id.txt_hue_setup);
        txt.setText(hue.getSetupText());

        Button connectButton = findViewById(R.id.button_connect_bridge);
        connectButton.setText(hue.isConnected() ? R.string.button_connect_new : R.string.button_connect);

        Button resetButton = findViewById(R.id.button_reset);
        resetButton.setVisibility(hue.isSetup() ? View.VISIBLE : View.GONE);

        Button greenLightsButton = findViewById(R.id.button_green_lights);
        greenLightsButton.setVisibility(hue.isSetup() ? View.VISIBLE : View.GONE);

        Button playVariousEffectsButton = findViewById(R.id.button_play_various_effects);
        playVariousEffectsButton.setVisibility(hue.isSetup() ? View.VISIBLE : View.GONE);
    }

    private void setGroups() {
        TextView txt = findViewById(R.id.txt_select_group);
        Spinner groupSpinner = findViewById(R.id.spinner_select_group);

        if (hue.isSetup()) {
            Bridge bridge = hue.getBridge();
            GroupVector groups = bridge.GetGroups();

            List<String> arraySpinner = createGroupsSpinnerContent(groups);
            String selected = bridge.IsValidGroupSelected() ? bridge.GetGroup().GetName() : "";
            initializeSpinner(groupSpinner, arraySpinner, selected);
            createGroupsSpinnerListener(groupSpinner);

            groupSpinner.setVisibility(View.VISIBLE);
            txt.setVisibility(View.VISIBLE);
        } else {
            groupSpinner.setVisibility(View.GONE);
            txt.setVisibility(View.GONE);
        }
    }

    private List<String> createGroupsSpinnerContent(GroupVector groups) {
        List<String> groupArray = new ArrayList<>();
        if (groups.size() > 0) {
            for (int i = 0; i < groups.size(); i++) {
                groupArray.add(groups.get(i).GetName());
            }
        } else {
            groupArray.add(getString(R.string.no_group));
        }
        return groupArray;
    }

    private void createGroupsSpinnerListener(Spinner groupSpinner) {
        groupSpinner.setOnItemSelectedListener(new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parentView, View selectedItemView, int position, long id) {
                Bridge bridge = hue.getBridge();
                if (position < bridge.GetGroups().size()) {
                    Group newGroup = bridge.GetGroups().get(position);
                    if (!bridge.IsValidGroupSelected() || !newGroup.GetId().equals(bridge.GetGroup().GetId())) {
                        hue.selectGroup(newGroup);
                    }
                }
            }

            @Override
            public void onNothingSelected(AdapterView<?> parentView) {
            }
        });
    }

    private void initializeSpinner(Spinner spinner, List<String> arraySpinner, String selected) {
        StringArrayWithUnselectedAdapter adapter = new StringArrayWithUnselectedAdapter(this,
                android.R.layout.simple_spinner_item, arraySpinner);
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        spinner.setAdapter(adapter);

        spinner.setSelection(adapter.getPosition(selected));
    }

    class StringArrayWithUnselectedAdapter extends ArrayAdapter<String> {
        public StringArrayWithUnselectedAdapter(Context context, int resource, List<String> objects) {
            super(context, resource, objects);
            this.add("");
        }

        @Override
        public int getCount() {
            return super.getCount() - 1;
        }
    }
}
