package com.example.testnfc;

import androidx.appcompat.app.AppCompatActivity;

import android.content.Intent;
import android.graphics.Color;
import android.nfc.NfcAdapter;
import android.nfc.Tag;
import android.os.Bundle;
import android.text.Editable;
import android.text.TextWatcher;
import android.util.Log;
import android.view.Gravity;
import android.view.KeyEvent;
import android.view.View;
import android.view.WindowManager;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Spinner;
import android.widget.TextView;
import android.widget.Toast;

import static com.example.testnfc.NFCCommand.SendReadSingleBlockCommand;

public class ControlActivity extends AppCompatActivity {

    private static final String DBG_NFC = "DEBUG_NFC";

    private static final String CMD_WRITE_CFG = "WRITE_CFG";
    private static final String NEW_CFG = "NEW_CFG";

    final String NFC_TAG = "NFC_TAG";
    final String TAG_ID = "TAG_ID";

    private TextView mTLogID;
    private Spinner mSpinnerModes;
    private EditText mControlInterval;
    private EditText mTopTemperatureLevel;
    private EditText mBottomTemperatureLevel;

    private byte   tlog_current_mode, tlog_new_mode;
    private byte   tlog_current_interval, tlog_new_interval;
    private byte   tlog_current_toplevel, tlog_new_toplevel;
    private byte   tlog_current_bottomlevel, tlog_new_bottomlevel;

    private boolean tlog_current_mode_changed = false;
    private boolean tlog_current_interval_changed = false;
    private boolean tlog_current_toplevel_changed = false;
    private boolean tlog_current_bottomlevel_changed = false;

    private Button  mButtonSaveCfg;


    private final String[] tlogModes = { "Не активирована", "Контроль температуры", "Остановлена", "Заблокирована"};

    /**************************************************************************************************************/
    /**************************************************************************************************************/
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        getSupportActionBar().hide(); // убираем заголовок приложения
        getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);

        setContentView(R.layout.activity_control);

        mTLogID = (TextView) findViewById(R.id.tlogID);
        mControlInterval = (EditText) findViewById(R.id.control_interval);
        mTopTemperatureLevel = (EditText) findViewById(R.id.topLevel);
        mBottomTemperatureLevel = (EditText) findViewById(R.id.bottomLevel);

        mButtonSaveCfg = (Button) findViewById(R.id.btnSaveCfg);

        mButtonSaveCfg.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                Intent return_data = new Intent();

                byte[] new_cfg = new byte[]{tlog_new_mode, tlog_new_interval, tlog_new_toplevel, tlog_new_bottomlevel};

                Log.d(DBG_NFC, String.format("Send %d %d %d %d", new_cfg[0], new_cfg[1], new_cfg[2], new_cfg[3]));

                return_data.putExtra(CMD_WRITE_CFG, true);
                return_data.putExtra(NEW_CFG, new_cfg);
                setResult(RESULT_OK, return_data);

                finish();
            }
        });

        setParameterFieldsListeners();


        Intent intent = getIntent();
        Tag tagFromIntent = intent.getParcelableExtra(NFC_TAG);
        byte[] TLogID = (byte[]) intent.getByteArrayExtra(TAG_ID);

        mTLogID.setText(String.format("%02X %02X %02X %02X %02X %02X %02X %02X",
                TLogID[9], TLogID[8], TLogID[7], TLogID[6], TLogID[5], TLogID[4], TLogID[3], TLogID[2]));

        // выбор режима работы метки
        mSpinnerModes = (Spinner) findViewById(R.id.spinnerModes);
        // Создаем адаптер ArrayAdapter с помощью массива строк и стандартной разметки элемета spinner
        ArrayAdapter<String> adapter = new ArrayAdapter<String>(this, R.layout.spinner_item, tlogModes);
        // Определяем разметку для использования при выборе элемента
        adapter.setDropDownViewResource(android.R.layout.simple_spinner_dropdown_item);
        // Применяем адаптер к элементу spinner
        mSpinnerModes.setAdapter(adapter);


        AdapterView.OnItemSelectedListener itemSelectedListener = new AdapterView.OnItemSelectedListener() {
            @Override
            public void onItemSelected(AdapterView<?> parent, View view, int position, long id) {

                // Получаем выбранный объект
//                String item = (String)parent.getItemAtPosition(position);

                if(position != tlog_current_mode) {
                    ((TextView) parent.getChildAt(0)).setTextColor(Color.RED);
                    tlog_current_mode_changed = true;
                    tlog_new_mode = (byte) position;
                }
                else {
                    tlog_current_mode_changed = false;
                    tlog_new_mode = tlog_current_mode;
                }

                SetBtnSaveCfgVisibility();
            }

            @Override
            public void onNothingSelected(AdapterView<?> parent) {
            }
        };

        mSpinnerModes.setOnItemSelectedListener(itemSelectedListener);


        readTLogCfg(new TLogData(tagFromIntent, TLogID));
    }


    /**************************************************************************************************************/
    /**************************************************************************************************************/
    private void SetBtnSaveCfgVisibility() {
//        if(tlog_current_mode_changed ||
//                tlog_current_interval_changed ||
//                tlog_current_toplevel_changed ||
//                tlog_current_bottomlevel_changed) {
//            mButtonSaveCfg.setVisibility(View.VISIBLE);
//        }
//        else {
//            mButtonSaveCfg.setVisibility(View.INVISIBLE);
//        }
    }


    /**************************************************************************************************************/
    /**************************************************************************************************************/
    private void setParameterFieldsListeners() {
        //========================================================================================//
        mControlInterval.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {
            }

            @Override
            public void afterTextChanged(Editable editable) {
                // максимальный интервал 1 час = 120 30-секундных интервалов
                try {
                    if (editable.length() > 0) {
                        int interval = Integer.parseInt(editable.toString());

                        if (interval > 120)
                            mControlInterval.setText(String.valueOf(120));
                    } else {
                        mControlInterval.setText(String.valueOf(0));
                    }
                } catch (NumberFormatException e) { }
            }
        });

        mControlInterval.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean b) {
                if(!b) {
                    // code to execute when EditText loses focus
                    if(tlog_current_interval != Integer.parseInt(mControlInterval.getText().toString())){
                        tlog_current_interval_changed = true;
                        tlog_new_interval = (byte) Integer.parseInt(mControlInterval.getText().toString());
                    }
                    else {
                        tlog_current_interval_changed = false;
                        tlog_new_interval = tlog_current_interval;
                    }

                    SetBtnSaveCfgVisibility();
                }
            }
        });

        //========================================================================================//
        mTopTemperatureLevel.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                // диапазон температур -40 ... +85
                try{
                    int level =Integer.parseInt(editable.toString());

                    if(level < -40)
                        mTopTemperatureLevel.setText(String.valueOf(-40));

                    if(level > 85)
                        mTopTemperatureLevel.setText(String.valueOf(85));
                } catch (NumberFormatException e) { }
            }
        });


        mTopTemperatureLevel.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean b) {
                if(!b) {
                    // code to execute when EditText loses focus
                    if(tlog_current_toplevel != Integer.parseInt(mTopTemperatureLevel.getText().toString())){
                        tlog_current_toplevel_changed = true;
                        tlog_new_toplevel = (byte) Integer.parseInt(mTopTemperatureLevel.getText().toString());
                    }
                    else {
                        tlog_current_toplevel_changed = false;
                        tlog_new_toplevel = tlog_current_toplevel;
                    }

                    SetBtnSaveCfgVisibility();
                }
            }
        });

        //========================================================================================//
        mBottomTemperatureLevel.addTextChangedListener(new TextWatcher() {
            @Override
            public void beforeTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void onTextChanged(CharSequence charSequence, int i, int i1, int i2) {

            }

            @Override
            public void afterTextChanged(Editable editable) {
                // диапазон температур -40 ... +85
                // диапазон температур -40 ... +85
                try{
                    int level =Integer.parseInt(editable.toString());

                    if(level < -40)
                        mBottomTemperatureLevel.setText(String.valueOf(-40));

                    if(level > 85)
                        mBottomTemperatureLevel.setText(String.valueOf(85));
                } catch (NumberFormatException e) { }
            }
        });

        mBottomTemperatureLevel.setOnFocusChangeListener(new View.OnFocusChangeListener() {
            @Override
            public void onFocusChange(View view, boolean b) {
                if(!b) {
                    // code to execute when EditText loses focus
                    if(tlog_current_bottomlevel != Integer.parseInt(mBottomTemperatureLevel.getText().toString())){
                        tlog_current_bottomlevel_changed = true;
                    }
                    else {
                        tlog_current_bottomlevel_changed = false;
                    }

                    SetBtnSaveCfgVisibility();
                }
            }
        });

        mBottomTemperatureLevel.setOnKeyListener(new View.OnKeyListener() {
            @Override
            public boolean onKey(View view, int i, KeyEvent keyEvent) {

                if((keyEvent.getAction() == KeyEvent.ACTION_DOWN) && (i == KeyEvent.KEYCODE_ENTER)) {
                    if(tlog_current_bottomlevel != Integer.parseInt(mBottomTemperatureLevel.getText().toString())){
                        tlog_current_bottomlevel_changed = true;
                        tlog_new_bottomlevel = (byte) Integer.parseInt(mBottomTemperatureLevel.getText().toString());
                    }
                    else {
                        tlog_current_bottomlevel_changed = false;
                        tlog_new_bottomlevel = tlog_current_bottomlevel;
                    }

                    SetBtnSaveCfgVisibility();
                }

                return false;
            }
        });
    }



    /**************************************************************************************************************/
    /**************************************************************************************************************/
    private void readTLogCfg(TLogData aTLogData) {

        byte[] cfg_address = {0x00, 0x00};
        byte[] cfg_block;
        cfg_block = SendReadSingleBlockCommand(aTLogData.getTag(), cfg_address);

        if ((cfg_block.length != 5) || (cfg_block[0] != 0)) {
            Toast.makeText(this, "Ошибка чтения данных!", Toast.LENGTH_SHORT).show();

            finish();   // закрываем активность
        }

//        Log.d(DBG_NFC, String.format("%d %d %d %d %d", cfg_block[0], cfg_block[1], cfg_block[2], cfg_block[3], cfg_block[4]));

        // формат ответа:
        // cfg_block[0] - Response flags => 0 - no errors
        // cfg_block[1] - current Mode:
        //                  режимы работы T-LOG (TLOG_CFG.mode)
        //                  MODE_IDLE	    0	// после включения питания, до команды START CONTROL
        //                  MODE_CONTROL	1	// режим контроля температуры
        //                  MODE_STOPPED	2   // остановлен командой STOP CONTROL или при возникновении исключения
        //                  MODE_BLOCKED	3	// заблокирована командой BLOCK
        // cfg_block[2] - Частота опроса датчика температуры в 30 с интервалах
        // cfg_block[3] - Верхняя допустимая температура
        // cfg_block[4] - Нижняя допустимая температура

        //------------------------------------------------------------------------------//
        // current mode
        tlog_current_mode = cfg_block[1];

        if(tlog_current_mode < 0 || tlog_current_mode > 3) {
            tlog_current_mode = tlog_new_mode = 0;
            tlog_current_mode_changed = true;
        }

        mSpinnerModes.setSelection(tlog_current_mode);

        //------------------------------------------------------------------------------//
        // control interval * 30 c
        tlog_current_interval = cfg_block[2];

        if(tlog_current_interval < 0 || tlog_current_interval > 120) {
            tlog_current_interval = tlog_new_interval = 0;
            tlog_current_interval_changed = true;
        }

        mControlInterval.setText(String.valueOf(tlog_current_interval));

        //------------------------------------------------------------------------------//
        // top temperature level
        tlog_current_toplevel = cfg_block[3];

        if(tlog_current_toplevel < -40 || tlog_current_toplevel > 85) {
            tlog_current_toplevel = tlog_new_toplevel = 0;
            tlog_current_toplevel_changed = true;
        }

        mTopTemperatureLevel.setText(String.valueOf(tlog_current_toplevel));

        //------------------------------------------------------------------------------//
        // bottom temperature level
        tlog_current_bottomlevel = cfg_block[4];

        if(tlog_current_bottomlevel < -40 || tlog_current_bottomlevel > 85) {
            tlog_current_bottomlevel = tlog_new_bottomlevel = 0;
            tlog_current_bottomlevel_changed = true;
        }

        mBottomTemperatureLevel.setText(String.valueOf(tlog_current_bottomlevel));

        if(tlog_current_mode_changed ||
                tlog_current_interval_changed ||
                tlog_current_toplevel_changed ||
                tlog_current_bottomlevel_changed) {

            SetBtnSaveCfgVisibility();

            Toast toast = Toast.makeText(this, new String("Обнаружена некорректная конфигурация!\nРекомендуется перезаписать"), Toast.LENGTH_LONG);
            TextView v = (TextView) toast.getView().findViewById(android.R.id.message);
            if( v != null) v.setGravity(Gravity.CENTER);
            toast.show();
        }
    }
}