package com.example.testnfc;

import androidx.annotation.Nullable;
import androidx.appcompat.app.AppCompatActivity;

import android.app.Activity;
import android.app.PendingIntent;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.graphics.Color;
import android.nfc.NfcAdapter;
import android.nfc.Tag;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.os.SystemClock;
import android.os.VibrationEffect;
import android.provider.Settings;
import android.util.Log;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;
import android.webkit.WebSettings;
import android.webkit.WebView;
import android.widget.Button;

import android.os.Vibrator;
import android.widget.ImageView;
import android.widget.Toast;

import com.google.android.material.tabs.TabLayout;

import static com.example.testnfc.NFCCommand.SendInventoryCommand;
import static com.example.testnfc.NFCCommand.SendReadSingleBlockCommand;
import static com.example.testnfc.NFCCommand.SendWriteSingleBlockCommand;

public class MainActivity extends AppCompatActivity {

    private static final String DBG_NFC = "DEBUG_NFC";


    private static final String CMD_WRITE_CFG = "WRITE_CFG";
    private static final String NEW_CFG = "NEW_CFG";
    private static final int CONTROL_TLOG = 0;

    final String NFC_TAG = "NFC_TAG";
    final String TAG_ID = "TAG_ID";

    private Button      nBtnSwitchOnNfc;
//    private WebView     nNFC_search;
    private ImageView   nNFC_search;
    private Button      nTLogSearchMessage;

    private boolean bWriteCfg = false;  // false - считываем данные
                                        // true - записываем конфигурацию

    private byte[] new_cfg;


    private void Buzzer(int duration) {
        Vibrator v = (Vibrator) getSystemService(Context.VIBRATOR_SERVICE);
        // Vibrate for duration milliseconds
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            v.vibrate(VibrationEffect.createOneShot(duration, VibrationEffect.DEFAULT_AMPLITUDE));
        } else {
            //deprecated in API 26
            v.vibrate(duration);
        }
    }



    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getSupportActionBar().hide(); // убираем заголовок приложения
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.activity_main);

        nNFC_search = findViewById(R.id.tlog_search);
        nTLogSearchMessage = findViewById(R.id.tlog_search_message);

        // Check for available NFC Adapter
        PackageManager packetManager = getPackageManager();
        if(!packetManager.hasSystemFeature(PackageManager.FEATURE_NFC)) {
            Toast.makeText(this, "Данная система не поддерживает NFC", Toast.LENGTH_SHORT).show();

            Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                public void run() {
                    /*code*/
                    finish();
                }
            }, 2000); //specify the number of milliseconds
        }

        if(NfcAdapter.getDefaultAdapter(this) == null) {
            Toast.makeText(this, "Не найден системный адаптер NFC", Toast.LENGTH_SHORT).show();

            Handler handler = new Handler();
            handler.postDelayed(new Runnable() {
                public void run() {
                    /*code*/
                    finish();
                }
            }, 2000); //specify the number of milliseconds
        }

        nBtnSwitchOnNfc = (Button) findViewById(R.id.btn_switch_on_nfc);

        nBtnSwitchOnNfc.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                startActivity(new Intent(Settings.ACTION_NFC_SETTINGS));
            }
        });
    }

    @Override
    protected void onResume() {
        super.onResume();

        if (NfcAdapter.getDefaultAdapter(this).isEnabled()) {
            PendingIntent pendingIntent = PendingIntent.getActivity(this,
                    0,
                    new Intent(this, getClass()).addFlags(Intent.FLAG_ACTIVITY_SINGLE_TOP),
                    0);

            IntentFilter ndef = new IntentFilter(NfcAdapter.ACTION_TECH_DISCOVERED);
            IntentFilter[] mFilters = new IntentFilter[] {ndef,};
            String[][] mTechLists = new String[][] { new String[] { android.nfc.tech.NfcV.class.getName() } };

            NfcAdapter.getDefaultAdapter(this).enableForegroundDispatch(this, pendingIntent, mFilters, mTechLists);

            nBtnSwitchOnNfc.setVisibility(View.GONE);

            nNFC_search.setVisibility(View.VISIBLE);
            nTLogSearchMessage.setVisibility(View.VISIBLE);

        }
        else {
            nBtnSwitchOnNfc.setVisibility(View.VISIBLE);
            nNFC_search.setVisibility(View.GONE);
            nTLogSearchMessage.setVisibility(View.GONE);
        }
    }


    @Override
    protected void onPause() {
        super.onPause();

        if(NfcAdapter.getDefaultAdapter(this).isEnabled()) {
            NfcAdapter.getDefaultAdapter(this).disableForegroundDispatch(this);
        }
        else{
            nBtnSwitchOnNfc.setVisibility(View.VISIBLE);
            nNFC_search.setVisibility(View.GONE);
            nTLogSearchMessage.setVisibility(View.GONE);
        }
    }

    @Override
    public void onNewIntent(Intent intent) {
        super.onNewIntent(intent);
        String action = intent.getAction();

        if(NfcAdapter.ACTION_TECH_DISCOVERED.equals(action)) {
            Tag tagFromIntent = intent.getParcelableExtra(NfcAdapter.EXTRA_TAG);

            byte[] TLogAnswer;

            TLogAnswer = SendInventoryCommand(tagFromIntent);
//            Toast.makeText(this, new String(String.valueOf(TLogAnswer.length)), Toast.LENGTH_SHORT).show();

//            for(byte answer: TLogAnswer)
//                Log.d(DBG_NFC, String.format("%02X", answer));

            if((TLogAnswer[0]!=0) || (TLogAnswer[1]!=-1)) {
                nTLogSearchMessage.setTextColor(Color.rgb(200,0,0));
                nTLogSearchMessage.setText(String.format("Обнаружна неизвестная метка\n Повторите попытку"));
            }
            else {

                if(bWriteCfg == true) {
                    // необходимо записать конфигурацию в метку
                    byte[] cfg_address = {0x00, 0x00};
//                    new_cfg = new byte[]{4, 3, 2, 1};   // 4 incorrect value of mode
                    byte[] result = SendWriteSingleBlockCommand(tagFromIntent, cfg_address, new_cfg);

                    if(result[0]!=0) {
                        nTLogSearchMessage.setTextColor(Color.rgb(200,0,0));
                        nTLogSearchMessage.setText(String.format("Ошибка записи конфигурации\n Повторите попытку"));

                        return;
                    }
                    else
                        Toast.makeText(this, new String("Конфигурация записана успешно!"), Toast.LENGTH_LONG).show();

                }

                nTLogSearchMessage.setTextColor(Color.WHITE);
                nTLogSearchMessage.setText(String.format("Поднесите смартфон к метке T-LOG"));

                // здесь вызвать Activity с информацией по T-Log метке
                Intent intentControlActivity = new Intent(MainActivity.this, ControlActivity.class);
                intentControlActivity.putExtra(NFC_TAG, tagFromIntent);
                intentControlActivity.putExtra(TAG_ID, TLogAnswer);
//                startActivity(intentControlActivity);
                startActivityForResult(intentControlActivity, CONTROL_TLOG);
            }
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, @Nullable Intent data) {
        super.onActivityResult(requestCode, resultCode, data);

        if(resultCode != Activity.RESULT_CANCELED) {
            if(requestCode == CONTROL_TLOG) {
                if(data == null)
                    return;

                bWriteCfg = data.getBooleanExtra(CMD_WRITE_CFG, false);
                new_cfg = data.getByteArrayExtra(NEW_CFG);

                Log.d(DBG_NFC, String.format("Received %d %d %d %d", new_cfg[0], new_cfg[1], new_cfg[2], new_cfg[3]));

                nTLogSearchMessage.setText(String.format("Для записи конфиграции поднесите смартфон к метке T-LOG"));
            }

        }
    }
}