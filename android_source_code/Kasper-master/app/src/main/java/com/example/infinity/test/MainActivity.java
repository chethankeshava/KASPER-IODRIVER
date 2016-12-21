package com.example.infinity.test;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.List;
import java.util.Map;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.os.Build;
import android.os.Bundle;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

public class MainActivity extends Activity {
    private static final String TAG = "Kasper";

    TextView result;
    private String address;
    Handler h;

    final int RECIEVE_MESSAGE = 1;        // Status  for Handler
    private BluetoothAdapter btAdapter = null;
    public BluetoothSocket btSocket = null;
    private StringBuilder sb = new StringBuilder();

    public ConnectedThread mConnectedThread;
    private BluetoothDevice deviceName;

    // SPP UUID service
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    /** Called when the activity is first created. */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);

        setContentView(R.layout.activity_main);

        deviceName = getIntent().getParcelableExtra("device");
        address = deviceName.getAddress();
        TextView tv = (TextView) findViewById(R.id.deviceName);
        tv.setText(deviceName.getName());
        result = (TextView) findViewById(R.id.result);
        final EditText et = (EditText) findViewById(R.id.box);

        h = new Handler() {
            public void handleMessage(android.os.Message msg) {
                switch (msg.what) {
                    case RECIEVE_MESSAGE:                                                   // if receive massage
                        byte[] readBuf = (byte[]) msg.obj;
                        String strIncom = new String(readBuf, 0, msg.arg1);
                        Log.d(TAG, strIncom);
                        sb.append(strIncom);
                        //result.setText(sb.toString() + "\n");
                        int endOfLineIndex = sb.indexOf("$");
                        Log.d(TAG,endOfLineIndex+"");
                        if (endOfLineIndex > 0) {                                            // if end-of-line,
                            String sbprint = sb.substring(0, endOfLineIndex);               // extract string
                            sb.delete(0, sb.length());
                            if(sbprint.toString().contains(","))
                            {
                                String parts[] = sbprint.toString().split(",");
                                if(parts[0].toString().equalsIgnoreCase("loc"))
                                {
                                    String lat = parts[1].toString();
                                    String lon = parts[2].toString();
                                    if(lat.length()>0 && lon.length()>0)
                                    {
                                        try {
                                            btSocket.close();
                                        } catch (IOException e) {
                                            e.printStackTrace();
                                        }
                                        Intent in = new Intent(MainActivity.this,MapActivity.class);
                                        in.putExtra("lat", lat);
                                        in.putExtra("lon", lon);
                                        in.putExtra("address", address);
                                        startActivity(in);
                                    }
                                    else
                                    {
                                        result.setText("Location not received");
                                    }
                                }
                                else
                                {
                                    showToast("No Location");
                                    result.setText("Data from Mac: " + sbprint);
                                }
                            }
                            else
                            {
                                showToast("No ,");
                                result.setText("Data from Mac: " + sbprint);
                            }
                        }
                        //Log.d(TAG, "...String:"+ sb.toString() +  "Byte:" + msg.arg1 + "...");
                        break;
                }
            };
        };



        btAdapter = BluetoothAdapter.getDefaultAdapter();       // get Bluetooth adapter
        checkBTState();

        Log.d(TAG, "...onCreate - try connect...");

        // Set up a pointer to the remote node using it's address.
        BluetoothDevice device = btAdapter.getRemoteDevice(address);

        try {
            btSocket = createBluetoothSocket(device);

        } catch (IOException e) {
            errorExit("Fatal Error", "In onResume() and socket create failed: " + e.getMessage() + ".");
        }

        // Discovery is resource intensive.  Make sure it isn't going on
        // when you attempt to connect and pass your message.
        btAdapter.cancelDiscovery();

        // Establish the connection.  This will block until it connects.
        Log.d(TAG, "...Connecting...");
        try {
            btSocket.connect();
            Log.d(TAG, "....Connection ok...");
        } catch (IOException e) {
            Log.d(TAG, "....Entering Catch...");
            e.printStackTrace();
            try {
                Log.d(TAG, "....Closing Socket...");
                btSocket.close();
            } catch (IOException e2) {
                Log.d(TAG, "....Fatal Error...");
                errorExit("Fatal Error", "In onResume() and unable to close socket during connection failure" + e2.getMessage() + ".");
            }
        }

        // Create a data stream so we can talk to server.
        Log.d(TAG, "...Create Socket...");

        mConnectedThread = new ConnectedThread(btSocket);
        mConnectedThread.start();


        Button bt = (Button) findViewById(R.id.send);
        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mConnectedThread.write(et.getText().toString());
                et.getText().clear();
            }
        });
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    public void showToast(String s)
    {
        Toast.makeText(getApplicationContext(), s.toString(),Toast.LENGTH_SHORT).show();
    }

    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        if(Build.VERSION.SDK_INT >= 10){
            try {
                final Method  m = device.getClass().getMethod("createInsecureRfcommSocketToServiceRecord", new Class[] { UUID.class });
                return (BluetoothSocket) m.invoke(device, MY_UUID);
            } catch (Exception e) {
                Log.e(TAG, "Could not create Insecure RFComm Connection",e);
            }
        }
        return  device.createRfcommSocketToServiceRecord(MY_UUID);
    }


    private void checkBTState() {
        // Check for Bluetooth support and then check to make sure it is turned on
        // Emulator doesn't support Bluetooth and will return null
        if(btAdapter==null) {
            errorExit("Fatal Error", "Bluetooth not support");
        } else {
            if (btAdapter.isEnabled()) {
                Log.d(TAG, "...Bluetooth ON...");
            } else {
                //Prompt user to turn on Bluetooth
                Intent enableBtIntent = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
                startActivityForResult(enableBtIntent, 1);
            }
        }
    }

    private void errorExit(String title, String message){
        Toast.makeText(getBaseContext(), title + " - " + message, Toast.LENGTH_LONG).show();
        finish();
    }

    public class ConnectedThread extends Thread {
        private final InputStream mmInStream;
        private final OutputStream mmOutStream;

        public ConnectedThread(BluetoothSocket socket) {
            InputStream tmpIn = null;
            OutputStream tmpOut = null;

            // Get the input and output streams, using temp objects because
            // member streams are final
            try {
                tmpIn = socket.getInputStream();
                tmpOut = socket.getOutputStream();
            } catch (IOException e) {
                Log.d(TAG, "..IOException : " + e.getMessage() + "...");
            }
            mmInStream = tmpIn;
            mmOutStream = tmpOut;
        }

        public void run() {
            byte[] buffer = new byte[256];  // buffer store for the stream
            int bytes; // bytes returned from read()

            // Keep listening to the InputStream until an exception occurs
            while (true) {
                try {
                    // Read from the InputStream
                    bytes = mmInStream.read(buffer); // Get number of bytes and message in "buffer"

                    h.obtainMessage(RECIEVE_MESSAGE, bytes, -1, buffer).sendToTarget();     // Send to message queue Handler
                } catch (IOException e) {
                    Log.d(TAG, "..Exception : " + e.getMessage() + "...");
                    //Log.d(TAG, Log.getStackTraceString(new Exception()));
                    break;
                }
            }
        }


        /* Call this from the main activity to send data to the remote device */
        public void write(String message) {
            Log.d(TAG, "...Data to send: " + message + "...");
            byte[] msgBuffer = message.getBytes();
            try {
                mmOutStream.write(msgBuffer);
            } catch (IOException e) {
                Log.d(TAG, "...Error data send: " + e.getMessage() + "...");
            }
        }
    }
}
