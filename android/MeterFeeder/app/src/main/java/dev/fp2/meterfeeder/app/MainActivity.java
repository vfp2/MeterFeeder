package dev.fp2.meterfeeder.app;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.LinearLayout;
import android.widget.TextView;
import android.widget.Toast;

import androidx.appcompat.app.AppCompatActivity;

import dev.fp2.meterfeeder.MeterFeeder;
import dev.fp2.meterfeeder.MeterFeederException;

public class MainActivity extends AppCompatActivity {
    LinearLayout textViewsLayout;
    TextView textViews[];

    boolean isMEDsInitialized = false;
    int numberMEDs = 0;
    String[] meds;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);
        textViewsLayout = (LinearLayout) findViewById(R.id.text_views_layout);

        Button refreshBtn = (Button) findViewById(R.id.refresh_btn);
        refreshBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                refreshMEDs();
            }
        });

        initMEDs();
        refreshMEDs();
    }

    void initMEDs() {
        try {
            if (!isMEDsInitialized) {
                String initStatus = MeterFeeder.MF_Initialize(this);
                if (initStatus != null && !initStatus.isEmpty()) {
                    Toast.makeText(this, initStatus, Toast.LENGTH_LONG).show();
                    return;
                }

                // Create a text view for each connected generator (MED) to display it's output
                numberMEDs = MeterFeeder.MF_GetNumberGenerators();
                textViews = new TextView[numberMEDs];
                for (int i = 0; i < numberMEDs; i++) {
                    TextView tv = new TextView(this);
                    tv.setText(""+(i + 1));
                    textViewsLayout.addView(tv);
                    textViews[i] = tv;
                }

                // Keep a list of all the connected MEDs (Generators)
                meds = MeterFeeder.MF_GetListGenerators();
            }
        } catch (MeterFeederException mfe) {
            mfe.printStackTrace();
            Toast.makeText(this, mfe.getMessage(), Toast.LENGTH_LONG).show();
        }

        isMEDsInitialized = true;
    }

    void refreshMEDs() {
        try {
            if (!isMEDsInitialized) {
                initMEDs();
                if (!isMEDsInitialized) {
                    return;
                }
            }

            for (int i = 0; i < meds.length; i++) {
                String medId = meds[i].substring(0, 8);
                byte bite = -1;
                int bite2 = bite;
                bite = MeterFeeder.MF_GetByte(medId);
                bite2 = bite & 0xff;
                textViews[i].setText(meds[i] + ": " + bite2);
            }
        } catch (MeterFeederException mfe) {
            mfe.printStackTrace();
            Toast.makeText(this, mfe.getMessage(), Toast.LENGTH_LONG).show();
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        MeterFeeder.MF_Shutdown();
    }
}