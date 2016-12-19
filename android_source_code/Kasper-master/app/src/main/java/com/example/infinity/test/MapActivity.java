package com.example.infinity.test;

import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothSocket;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.graphics.Color;
import android.os.Build;
import android.os.Handler;
import android.support.v4.app.FragmentActivity;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.Toast;

import com.google.android.gms.maps.CameraUpdateFactory;
import com.google.android.gms.maps.GoogleMap;
import com.google.android.gms.maps.OnMapReadyCallback;
import com.google.android.gms.maps.SupportMapFragment;
import com.google.android.gms.maps.model.BitmapDescriptorFactory;
import com.google.android.gms.maps.model.LatLng;
import com.google.android.gms.maps.model.Marker;
import com.google.android.gms.maps.model.MarkerOptions;

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.ObjectOutputStream;
import java.io.OutputStream;
import java.io.Serializable;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

import com.example.infinity.test.LatLong;
import com.google.android.gms.maps.model.PolylineOptions;

public class MapActivity extends FragmentActivity implements OnMapReadyCallback {

    public List<LatLong> navigate = new ArrayList<>();
    private GoogleMap mMap;
    private Button sendCoordinates,startButton, stopButton, connectButton;
    private static final String TAG = "Kasper";
    Handler h;
    final int RECIEVE_MESSAGE = 1;
    public ConnectedThread mConnectedThread;
    private BluetoothSocket btSocket = null;
    private String address;
    private BluetoothDevice deviceName;
    private BluetoothAdapter btAdapter = null;
    Marker locMarker = null;
    private StringBuilder sb = new StringBuilder();
    private static final UUID MY_UUID = UUID.fromString("00001101-0000-1000-8000-00805F9B34FB");

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_map);

        deviceName = getIntent().getParcelableExtra("device");
        address = deviceName.getAddress();
        sendCoordinates = (Button) findViewById(R.id.navigateButton);
        connectButton = (Button) findViewById(R.id.connectButton);
        startButton = (Button) findViewById(R.id.startButton);
        stopButton = (Button) findViewById(R.id.stopButton);

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
                                Log.d(TAG,parts[0] + "-" + parts[0].length());
                                if(parts[0].toString().trim().equalsIgnoreCase("loc"))
                                {
                                    String lat = parts[1].toString();
                                    String lon = parts[2].toString();
                                    if(lat.length()>0 && lon.length()>0)
                                    {
                                        LatLng coordinates = new LatLng(Double.parseDouble(lat), Double.parseDouble(lon));
                                        LatLong carLoc = new LatLong();
                                        carLoc.setLatitude(Double.parseDouble(lat));
                                        carLoc.setLongitude(Double.parseDouble(lon));
                                        navigate.add(carLoc);
                                        mMap.addMarker(new MarkerOptions().position(coordinates).title("Car Location"));
                                        mMap.moveCamera(CameraUpdateFactory.newLatLng(coordinates));
                                        mMap.animateCamera(CameraUpdateFactory.zoomTo(20));

                                    }

                                    else
                                    {
                                        showToast("Location not received");
                                    }
                                }
                                else if(parts[0].toString().trim().equalsIgnoreCase("ab"))
                                {
                                    Log.d(TAG,"Hi Dude");
                                    String lat = parts[1].toString();
                                    String lon = parts[2].toString();
                                    if(lat.length()>0 && lon.length()>0)
                                    {
                                        if(locMarker!=null)
                                        {
                                            locMarker.remove();
                                        }
                                        LatLng coordinates = new LatLng(Double.parseDouble(lat), Double.parseDouble(lon));
                                        LatLong carLoc = new LatLong();
                                        carLoc.setLatitude(Double.parseDouble(lat));
                                        carLoc.setLongitude(Double.parseDouble(lon));
                                        navigate.add(carLoc);
                                        locMarker = mMap.addMarker(new MarkerOptions().position(coordinates).title("New Car Location"));
                                        locMarker.setIcon(BitmapDescriptorFactory.fromBitmap(resizeMapIcons(String.valueOf(R.mipmap.dot),25,25)));
                                        mMap.moveCamera(CameraUpdateFactory.newLatLng(coordinates));
                                        mMap.animateCamera(CameraUpdateFactory.zoomTo(20));

                                    }

                                    else
                                    {
                                        showToast("Location not received");
                                    }
                                }
                                else
                                {
                                    showToast("No Location");
                                }
                            }
                            else
                            {
                                if(sbprint.toString().equalsIgnoreCase("ack"))
                                {
                                    startButton.setVisibility(View.VISIBLE);
                                }
                                else
                                {
                                    showToast("No Ack Recieved");
                                }
                            }
                        }
                        //Log.d(TAG, "...String:"+ sb.toString() +  "Byte:" + msg.arg1 + "...");
                        break;
                }
            };
        };




        SupportMapFragment mapFragment = (SupportMapFragment) getSupportFragmentManager().findFragmentById(R.id.map);
        mapFragment.getMapAsync(this);

        connectButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mConnectedThread.write("connect\n");
            }
        });

        sendCoordinates.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {


                if(navigate.size()>0) {

                    mConnectedThread.write(navigate.size() - 1+"");
                    for(int i=1;i<navigate.size();i++)
                    {
                        mConnectedThread.write(navigate.get(i).getLatitude() + "," + navigate.get(i).getLongitude());
                    }
                    mConnectedThread.write("\n");

                    PolylineOptions opt = new PolylineOptions();
                    opt.color( Color.parseColor( "#CC0000FF" ) );
                    opt.width( 5 );
                    opt.visible( true );

                    for ( LatLong locs : navigate )
                    {
                        opt.add( new LatLng( locs.getLatitude(),
                                locs.getLongitude() ) );
                    }

                    mMap.addPolyline(opt);
                }
                else
                {
                    Toast.makeText(getApplicationContext(), "No Coordinates Found", Toast.LENGTH_SHORT).show();
                }
            }
        });

        startButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mConnectedThread.write("1\n");
                stopButton.setVisibility(View.VISIBLE);
                startButton.setVisibility(View.GONE);
            }
        });

        stopButton.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                mConnectedThread.write("01\n");
                startButton.setVisibility(View.VISIBLE);
                stopButton.setVisibility(View.GONE);
            }
        });

    }

    public Bitmap resizeMapIcons(String iconName,int width, int height){
        Bitmap imageBitmap = BitmapFactory.decodeResource(getResources(),getResources().getIdentifier(iconName, "drawable", getPackageName()));
        Bitmap resizedBitmap = Bitmap.createScaledBitmap(imageBitmap, width, height, false);
        return resizedBitmap;
    }

    @Override
    protected void onDestroy() {
        super.onDestroy();
    }

    private void showToast(String s)
    {
        Toast.makeText(getApplicationContext(), s+"",Toast.LENGTH_SHORT).show();
    }

    @Override
    public void onMapReady(GoogleMap googleMap) {
        mMap = googleMap;

        btAdapter = BluetoothAdapter.getDefaultAdapter();       // get Bluetooth adapter
        checkBTState();

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
        runOnUiThread(new Runnable() {
            @Override
            public void run() {
                Log.d(TAG, "...Connecting...");
                try {

                    btSocket.connect();
                    Log.d(TAG, "....Connection ok...");
                    // Create a data stream so we can talk to server.
                    Log.d(TAG, "...Create Socket...");

                    mConnectedThread = new ConnectedThread(btSocket);
                    mConnectedThread.start();
                } catch (IOException e) {
                    Log.d(TAG, "....Entering Catch...");
                    e.printStackTrace();
                    try {
                        Log.d(TAG, "....Closing Socket...");
                        btSocket.close();
                    } catch (IOException e2) {
                        Log.d(TAG, "....Fatal Error...");
                        errorExit("Fatal Error", "close socket during connection failure" + e2.getMessage() + ".");
                    }
                }
            }
        });

        mMap.setOnMapClickListener(new GoogleMap.OnMapClickListener() {
            @Override
            public void onMapClick(LatLng point) {
                int position = 1;
                MarkerOptions marker = new MarkerOptions().position(
                        new LatLng(point.latitude, point.longitude)).title(position+"")
                        .icon(BitmapDescriptorFactory.defaultMarker(BitmapDescriptorFactory.HUE_BLUE));

                mMap.addMarker(marker);
                LatLong cood = new LatLong();
                cood.setLatitude(point.latitude);
                cood.setLongitude(point.longitude);
                navigate.add(cood);

                sendCoordinates.setVisibility(View.VISIBLE);
                position++;
            }
        });

    }


    private BluetoothSocket createBluetoothSocket(BluetoothDevice device) throws IOException {
        if(Build.VERSION.SDK_INT >= 10){
            try {
                final Method m = device.getClass().getMethod("createInsecureRfcommSocketToServiceRecord", new Class[] { UUID.class });
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

    public class ConnectedThread extends Thread implements Serializable {
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
