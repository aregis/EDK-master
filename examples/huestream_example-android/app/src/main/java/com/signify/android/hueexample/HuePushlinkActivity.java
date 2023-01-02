/*******************************************************************************
 Copyright (C) 2021 Signify Holding B.V.
 All Rights Reserved.
 *******************************************************************************/

package com.signify.android.hueexample;

import android.app.Activity;
import android.os.Bundle;
import android.os.CountDownTimer;
import android.widget.ProgressBar;

import com.lighting.huestream.FeedbackMessage;
import com.lighting.huestream.FeedbackMessage.Id;

public class HuePushlinkActivity extends Activity implements HueHandler.HueFeedbackListener {
    private ProgressBar pbar;
    private static final int MAX_TIME = 45;
    private HueHandler hue;
    private CountDownTimer countDownTimer;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_hue_pushlink);
        setTitle(R.string.txt_pushlink);

        hue = ((HueExampleApplication) getApplication()).getHue();

        hue.registerFeedbackListener(this);

        pbar = findViewById(R.id.countdownPB);
        pbar.setMax(MAX_TIME);
        countDownTimer = new CountDownTimer(MAX_TIME * 1000, 1000) {

            @Override
            public void onTick(long millisUntilFinished) {
                incrementProgress();
            }

            @Override
            public void onFinish() {
                hue.abortConnecting();
                finish();
            }
        };
        countDownTimer.start();

    }

    @Override
    protected void onStop(){
        hue.deregisterFeedbackListener(this);
        super.onStop();
    }

    public void incrementProgress() {
        pbar.incrementProgressBy(1);
    }
    
    @Override
    public void onDestroy() {
        hue.deregisterFeedbackListener(this);
        super.onDestroy();
    }

    @Override
    public void onFeedbackMessage(FeedbackMessage message) {
        if (message.GetId() == Id.ID_FINISH_AUTHORIZING_AUTHORIZED || message.GetId() == Id.ID_FINISH_AUTHORIZING_FAILED) {
            countDownTimer.cancel();
            finish();
        }
    }
}
