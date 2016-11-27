// This is the screen-1 that appears when the application is opened.

package com.example.infinity.test;

import android.Manifest;
import android.app.ListActivity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.os.Bundle;
import android.os.Parcelable;
import android.support.v4.app.ActivityCompat;
import android.support.v4.content.ContextCompat;
import android.util.Log;
import android.view.View;
import android.widget.AdapterView;
import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.ListView;
import android.widget.Toast;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.lang.reflect.Method;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.UUID;


import android.Manifest;
import android.app.ListActivity;
import android.bluetooth.BluetoothServerSocket;
import android.bluetooth.BluetoothSocket;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;

import android.content.pm.PackageManager;
import android.os.Build;
import android.os.Message;
import android.os.Parcelable;
import android.support.annotation.RequiresApi;
import android.support.v4.app.ActivityCompat;
import android.support.v4.app.Fragment;
import android.support.v4.app.FragmentActivity;

import android.os.Handler;
import android.support.v4.content.ContextCompat;
import android.support.v7.app.ActionBarActivity;

import java.net.Socket;
import java.util.UUID;

import android.support.v7.app.AlertDialog;
import android.support.v7.app.AppCompatActivity;
import android.os.Bundle;
import android.util.Log;
import android.view.View;
import android.widget.Adapter;
import android.widget.AdapterView;
import android.widget.Button;
import android.widget.Toast;

import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;

import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;

import android.graphics.Color;
import android.net.wifi.ScanResult;
import android.net.wifi.WifiManager;

import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;

import android.widget.ArrayAdapter;
import android.widget.Button;
import android.widget.EditText;
import android.widget.ListView;
import android.widget.TextView;
import android.widget.Toast;

import java.util.ArrayList;
import java.util.List;
import java.util.Set;

import java.lang.String;

import android.os.ParcelUuid;

import static android.net.Uri.parse;

public class FirstActivity extends ListActivity {

    private static final int REQUEST_COARSE_LOCATION = 900 ;
    private Button b1, b2, b3, b4;                             //Buttons on the first screen
    private BluetoothAdapter BA;
    private Set<BluetoothDevice> pairedDevices;
    //private ConnectThread connectThread;

    ListView lv;
    ListView lv2;

    ArrayList al = new ArrayList();                           // Array list for storing the list of devices.

    BluetoothServerSocket mmServerSocket;                     // Socket used for connection.

    ArrayList<BluetoothDevice> list = new ArrayList();
    ArrayList list1 = new ArrayList();
    List<BluetoothDevice> bluetoothList = new ArrayList<BluetoothDevice>();

    ArrayAdapter<String> adapter = null;


    private String name;

    private UUID my_uuid;                                     // UUID stands for universally unique identifier. It is a 128 bit number.
                                                              // for Bluetooth communication there is a fixed UUID. It usually represents some
                                                              // common service protocols that bluetooth device supports.

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_first);

        b1 = (Button) findViewById(R.id.button1);
        b2 = (Button) findViewById(R.id.button2);
        b3 = (Button) findViewById(R.id.button3);
        b4 = (Button) findViewById(R.id.button4);

        BA = BluetoothAdapter.getDefaultAdapter();
        lv = (ListView) findViewById(R.id.list_view_id);

        lv2 = getListView();

        adapter = new ArrayAdapter(getApplicationContext(), android.R.layout.simple_list_item_1, list1);

        setListAdapter(adapter);

        lv2.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> parent, View view, int position, long id) {
                showToast("Selected");
                Log.d("Device Selected: " , ""+bluetoothList.get(position));
                Intent in = new Intent(FirstActivity.this,MainActivity.class);
                in.putExtra("device", bluetoothList.get(position));
                startActivity(in);
            }
        });
    }

    // The Below function is called when the TURN ON button is pressed.
    public void Turn_ON_button(View view) {
        /* //testing google API working
    Intent intent = null, chooser = null;
        intent = new Intent(android.content.Intent.ACTION_VIEW);
        intent.setData(Uri.parse("geo:37.3403980,-121.8944450"));
        chooser = Intent.createChooser(intent, "Launch Map");
        startActivity(chooser);
        */
        if (!BA.isEnabled()) {
            Intent turnOn = new Intent(BluetoothAdapter.ACTION_REQUEST_ENABLE);
            startActivityForResult(turnOn, 0);
            Toast.makeText(getApplicationContext(), "Turned On", Toast.LENGTH_LONG).show();  // Toast method is used to display the text on screen for short/long duration to the user.
        } else {
            Toast.makeText(getApplicationContext(), "Already On", Toast.LENGTH_LONG).show();
        }
    }

    // The Below function is called when the TURN OFF button is pressed.
    public void Turn_OFF_button(View view) {

        BA.disable();
        Toast.makeText(getApplicationContext(), "Turned off", Toast.LENGTH_LONG).show();

    }

    // The Below function is called when the GET VISIBLE button is pressed.
    public void Get_Visible_button(View view) {

        Intent getVisible = new Intent(BluetoothAdapter.ACTION_REQUEST_DISCOVERABLE);
        startActivityForResult(getVisible, 0);
    }

    // The “List_Devices_button” method will query about the set of paired devices to see if the device is already known.
    // The Below function is called when the LIST DEVICES button is pressed.

    public void List_Devices_button(View view) {

        pairedDevices = BA.getBondedDevices(); // This is to see if the desired device is already known (This information comes from the API, that has the value stored in it)

        for (BluetoothDevice bt : pairedDevices) {
            list.add(bt);
            al.add(bt.getName());
        }
        //list.add(bt.getName() + "\n" + bt.getAddress());  // This line of code will get the name and address of the devices that were previously connected to this device (i.e. paired devices)

        Toast.makeText(getApplicationContext(), "Showing Paired Devices", Toast.LENGTH_SHORT).show();

        ArrayAdapter adapter2 = new ArrayAdapter(getApplicationContext(), android.R.layout.simple_list_item_1, al);
        lv.setAdapter(adapter2);

        lv.setOnItemClickListener(new AdapterView.OnItemClickListener() {
            @Override
            public void onItemClick(AdapterView<?> adapterView, View view, int i, long l) {
                Intent in = new Intent(FirstActivity.this, MainActivity.class);
                in.putExtra("device", list.get(i));
                startActivity(in);
            }
        });

    }
        // Below code shows how you can listen for a click on the button.
        /*
        b4.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {
                Toast.makeText(MainActivity.this, “Paired devices : “, Toast.LENGTH_SHORT).show();
            }
        });
        */

    // “The Find_Device_button” method will start the discovery of new available devices.
    // The Below function is called when the LIST DEVICES button is pressed.

    public void Find_Device_button(View view) {

        IntentFilter filter = new IntentFilter();

        filter.addAction(BluetoothDevice.ACTION_FOUND);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_STARTED);
        filter.addAction(BluetoothAdapter.ACTION_DISCOVERY_FINISHED);

        registerReceiver(mReceiver, filter);
        BA.startDiscovery();
    }

    private final BroadcastReceiver mReceiver = new BroadcastReceiver() {
        public void onReceive(Context context, Intent intent) {
            String action = intent.getAction();

            if (BluetoothAdapter.ACTION_DISCOVERY_STARTED.equals(action)) {
                //discovery starts, we can show progress dialog or perform other tasks
                showToast("job started");
            } else if (BluetoothAdapter.ACTION_DISCOVERY_FINISHED.equals(action)) {
                //discovery finishes, dismiss progress dialog
                for(int i =0; i < bluetoothList.size();i++){
                    //    Log.d(bluetoothList.get(i).getName(),bluetoothList.get(i).EXTRA_UUID.toString());
                }
                showToast("job finished");
            }
            if(BluetoothDevice.ACTION_UUID.equals(action)) {
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);
                Parcelable[] uuidExtra = intent.getParcelableArrayExtra(BluetoothDevice.EXTRA_UUID);
                for (int i=0; i<uuidExtra.length; i++) {
                    Log.d(" Device: " + device.getName() ,  uuidExtra[i].toString());
                }
            }
            else if (BluetoothDevice.ACTION_FOUND.equals(action)) {
                //bluetooth device found
                Log.d("Device"," found");
                BluetoothDevice device = intent.getParcelableExtra(BluetoothDevice.EXTRA_DEVICE);

//                list1.add(device.getName() + "|" + device.getAddress());

                /*
                final ArrayAdapter adapter2 = new ArrayAdapter<String>(this, android.R.layout.simple_list_item_1, list1);
                lv2.setAdapter(adapter2);
                */
//                Log.d("Device found",device.getName());

                //              showToast("Found device :" + device.getName());

                //             showToast("Found device :::" + list1); // for testing that values are coming in the list.

                bluetoothList.add(device);
                list1.add(device.getName() +"\n" + device.getAddress());
//                Log.d(device.getName(),device.getUuids().toString());
                adapter.notifyDataSetChanged();
//                List<String> list3 = new ArrayList<String>();
//                list3.add(device.getName()+"|"+
//                        device.getAddress());
//                adapter = new ArrayAdapter(getApplicationContext(), android.R.layout.simple_list_item_1, list3);
                //          lv2.setAdapter(adapter);
//
//                adapter.add(""+device.getName()+"|"+device.getAddress());
//                adapter.notifyDataSetChanged();
            }
        }
    };

    // Below method is used to pair with the available devices.
    private void pairDevice(BluetoothDevice device) {
        try {
            Method method = device.getClass().getMethod("createBond", (Class[]) null);
            method.invoke(device, (Object[]) null);
        } catch (Exception e) {
            e.printStackTrace();
        }
    }

    // Below showToast method is used to display messages on screen for the user.
    private void showToast(String message) {
        Toast.makeText(getApplicationContext(), message, Toast.LENGTH_SHORT).show();
    }


    // The below method is called for final clean up before the activity is destroyed.
    @Override
    public void onDestroy() {
        unregisterReceiver(mReceiver);

        super.onDestroy();
    }

    protected void checkLocationPermission() {
        if (ContextCompat.checkSelfPermission(this, Manifest.permission.ACCESS_COARSE_LOCATION)
                != PackageManager.PERMISSION_GRANTED) {

            ActivityCompat.requestPermissions(this,
                    new String[]{Manifest.permission.ACCESS_COARSE_LOCATION},
                    REQUEST_COARSE_LOCATION);
        }
    }

    @Override
    public void onRequestPermissionsResult(int requestCode,
                                           String permissions[], int[] grantResults) {
        switch (requestCode) {
            case REQUEST_COARSE_LOCATION: {
                if (grantResults.length > 0
                        && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
//                    BA.startDiscovery(); // --->
                    BA.startDiscovery();
                } else {
                    //TODO re-request
                }
                break;
            }
        }
    }
}