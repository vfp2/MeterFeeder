package dev.fp2.meterfeeder.app;

import android.os.Bundle;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;

import androidx.appcompat.app.AppCompatActivity;

import dev.fp2.meterfeeder.MeterFeeder;

public class MainActivity extends AppCompatActivity {
    TextView tvs[];

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);

        MeterFeeder.MF_Initialize(this);

//        tvs = new TextView[MeterFeeder.MF_GetNumberGenerators()];
        tvs = new TextView[2];
        tvs[0] = (TextView) findViewById(R.id.medtv1);
        tvs[1] = (TextView) findViewById(R.id.medtv1);

        Button refreshBtn = (Button) findViewById(R.id.refresh_btn);
        refreshBtn.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                refresh();
            }
        });

//        refresh();
    }

    void refresh() {
        String[] generators = MeterFeeder.MF_GetListGenerators();
        for (int i = 0; i < generators.length; i++) {
            tvs[i].setText(generators[i] + ": " + (MeterFeeder.MF_GetByte(generators[i].substring(0, 8)) & 0xff));
        }
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
        MeterFeeder.MF_Shutdown();
    }
}