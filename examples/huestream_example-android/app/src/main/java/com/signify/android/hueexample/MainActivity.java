/*******************************************************************************
 Copyright (C) 2021 Signify Holding B.V.
 All Rights Reserved.
 *******************************************************************************/

package com.signify.android.hueexample;

import android.app.Activity;
import android.content.Intent;
import android.os.Bundle;
import android.view.View;

public class MainActivity extends Activity {
    private HueHandler hue;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        hue = ((HueExampleApplication) getApplication()).getHue();
    }

    public void onHueSetupClicked(View view) {
        Intent intent = new Intent(this, HueSetupActivity.class);
        startActivity(intent);
    }

    @Override
    public void onStart() {
        super.onStart();
    }

    @Override
    public void onStop() {
        super.onStop();
    }

    @Override
    public void onSaveInstanceState(Bundle savedInstanceState) {
        super.onSaveInstanceState(savedInstanceState);
    }

}
