/*******************************************************************************
 Copyright (C) 2021 Signify Holding B.V.
 All Rights Reserved.
 *******************************************************************************/

package com.signify.android.hueexample;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.support.v4.content.ContextCompat;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ProgressBar;
import android.widget.TextView;

import com.lighting.huestream.ConnectResult;
import com.lighting.huestream.FeedbackMessage;
import com.lighting.huestream.FeedbackMessage.Id;

import java.util.regex.Pattern;

public class HueConnectActivity extends Activity implements HueHandler.HueFeedbackListener {

    private static final String IPADDRESS_PATTERN =
            "^([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
                    "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
                    "([01]?\\d\\d?|2[0-4]\\d|25[0-5])\\." +
                    "([01]?\\d\\d?|2[0-4]\\d|25[0-5])$";

    private HueHandler hue;
    private TextView connectText;
    private ProgressBar spinner;
    private EditText ipInput;
    private Button connectButton;
    private Pattern ipPattern;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hue_connect);

        connectText = findViewById(R.id.text_connecting);
        spinner = findViewById(R.id.connect_spinner);
        ipInput = findViewById(R.id.edit_ip);
        connectButton = findViewById(R.id.button_connect);

        hue = ((HueExampleApplication) getApplication()).getHue();

        if (hue.getConnectionResult() == ConnectResult.Busy) {
            hue.abortConnecting();
        }

        hue.registerFeedbackListener(this);

        if (hue.isConnected()) {
            hue.connectNewBridge();
        } else {
            hue.connect();
        }
    }

    public void onConnectPressed(View view) {
        if (hue.getConnectionResult() == ConnectResult.Busy) {
            hue.abortConnecting();
        } else {
            if (isValidIpEntered()) {
                String ip = ipInput.getText().toString();
                hue.connectWithIp(ip);
            } else {
                hue.connect();
            }
        }
    }

    @Override
    public void onFeedbackMessage(FeedbackMessage message) {
        if (HueConnectActivity.this.isFinishing())
            return;

        if (message.GetId() == Id.ID_PRESS_PUSH_LINK) {
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    startActivity(new Intent(HueConnectActivity.this, HuePushlinkActivity.class));
                }
            });
        } else if (message.GetId() == Id.ID_BRIDGE_NOT_FOUND ||
                   message.GetId() == Id.ID_NO_BRIDGE_FOUND) {
            final String txt = message.GetUserMessage();
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    connectText.setText(txt);
                    enableManualIPField();
                }
            });
        } else if (message.GetId() == Id.ID_INVALID_VERSION ||
                   message.GetId() == Id.ID_NO_GROUP_AVAILABLE ||
                   message.GetId() == Id.ID_INVALID_MODEL) {
            final String txt = message.GetUserMessage();
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    connectText.setText(txt);
                }
            });
        } else if (message.GetId() == Id.ID_DONE_COMPLETED ||
                   message.GetId() == Id.ID_SELECT_GROUP) {
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    //hue.setEnabled(true);
                    finish();
                }
            });
        } else if (message.GetId() == Id.ID_USERPROCEDURE_STARTED) {
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    setUIBusyConnecting();
                }
            });
        } else if (message.GetId() == Id.ID_USERPROCEDURE_FINISHED) {
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    setUIIdle();
                }
            });
        } else if (message.GetId() == Id.ID_DONE_ABORTED) {
            HueConnectActivity.this.runOnUiThread(new Runnable() {
                @Override
                public void run() {
                    connectText.setText("");
                }
            });
        }
    }

    @Override
    public void onDestroy() {
        super.onDestroy();
        hue.deregisterFeedbackListener(this);
    }

    private void setUIBusyConnecting() {
        connectText.setText(getString(R.string.connecting));
        spinner.setVisibility(View.VISIBLE);
        connectButton.setText(R.string.btn_abort);
        ipInput.setClickable(false);
        ipInput.setFocusable(false);
    }

    private void setUIIdle() {
        spinner.setVisibility(View.INVISIBLE);
        connectButton.setText(R.string.button_connect);
        ipInput.setClickable(true);
        ipInput.setFocusableInTouchMode(true);
    }

    private void enableManualIPField() {
        ipInput.setVisibility(View.VISIBLE);

        ipPattern = Pattern.compile(IPADDRESS_PATTERN);

        ipInput.addTextChangedListener(new TextWatcher() {

            public void afterTextChanged(Editable s) {
                String ip = ipInput.getText().toString();
                if (ip.isEmpty()) {
                    ipInput.getBackground().setTint(ContextCompat.getColor(getApplicationContext(),
                            android.R.color.background_light));
                } else if (ipPattern.matcher(ip).matches()) {
                    ipInput.getBackground().setTint(ContextCompat.getColor(getApplicationContext(),
                            android.R.color.holo_green_light));
                } else {
                    ipInput.getBackground().setTint(ContextCompat.getColor(getApplicationContext(),
                            android.R.color.holo_red_light));
                }
            }

            public void beforeTextChanged(CharSequence s, int start, int count, int after) {}

            public void onTextChanged(CharSequence s, int start, int before, int count) {}
        });
    }

    private boolean isValidIpEntered() {
        return ipInput.getVisibility() == View.VISIBLE &&
                ipPattern.matcher(ipInput.getText().toString()).matches();
    }

}