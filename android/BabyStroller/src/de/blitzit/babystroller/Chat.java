package de.blitzit.babystroller;

import java.util.HashMap;
import java.util.Map;
import java.util.UUID;

import android.app.Activity;
import android.bluetooth.BluetoothGattCharacteristic;
import android.bluetooth.BluetoothGattService;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.IBinder;
import android.util.Log;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.TextView;

import com.redbear.chat.Device;
import com.redbear.chat.RBLService;

public class Chat extends Activity {
	private final static String TAG = Chat.class.getSimpleName();

	public static final String EXTRAS_DEVICE = "EXTRAS_DEVICE";
	private Button btn = null;
	private String mDeviceName;
	private Button btnRetry;
	private String mDeviceAddress;
	private TextView label;
	private RBLService mBluetoothLeService;
	private Map<UUID, BluetoothGattCharacteristic> map = new HashMap<UUID, BluetoothGattCharacteristic>();

	private final ServiceConnection mServiceConnection = new ServiceConnection() {

		@Override
		public void onServiceConnected(ComponentName componentName,
				IBinder service) {
			mBluetoothLeService = ((RBLService.LocalBinder) service)
					.getService();
			if (!mBluetoothLeService.initialize()) {
				Log.e(TAG, "Unable to initialize Bluetooth");
				finish();
			}
			// Automatically connects to the device upon successful start-up
			// initialization.
			mBluetoothLeService.connect(mDeviceAddress);
		}

		@Override
		public void onServiceDisconnected(ComponentName componentName) {
			mBluetoothLeService = null;
		}
	};

	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();

			if (RBLService.ACTION_GATT_DISCONNECTED.equals(action)) {
				
			} else if (RBLService.ACTION_GATT_SERVICES_DISCOVERED
					.equals(action)) {
				getGattService(mBluetoothLeService.getSupportedGattService());
				Log.d(TAG, "Service discoverd!");
				instanciateConnection();
			} else if (RBLService.ACTION_DATA_AVAILABLE.equals(action)) {
				displayData(intent.getByteArrayExtra(RBLService.EXTRA_DATA));
			} else if (RBLService.ACTION_GATT_CONNECTED.equals(action)) {
				Log.d(TAG, "Service connected!");
//				instanciateConnection();
			}
		}
	};

	private Chat chat;

	protected void instanciateConnection() {
		BluetoothGattCharacteristic characteristic = map
				.get(RBLService.UUID_BLE_SHIELD_TX);

		String str = "on"; 
		byte b = 0x00;
		byte[] tmp = str.getBytes();
		byte[] tx = new byte[tmp.length + 1];
		tx[0] = b;
		for (int i = 1; i < tmp.length + 1; i++) {
			tx[i] = tmp[i - 1];
		}

		characteristic.setValue(tx);
		mBluetoothLeService.writeCharacteristic(characteristic);
		Log.d(TAG,"sending out data to instanciate connection!");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		chat = this;
		
		super.onCreate(savedInstanceState);
		setContentView(R.layout.second);

		
		btn = (Button) findViewById(R.id.send);
		btnRetry = (Button) findViewById(R.id.retry);
		label = (TextView) findViewById(R.id.result);
		
		btnRetry.setOnClickListener(new OnClickListener() {
			
			@Override
			public void onClick(View v) {
				unregisterReceiver(mGattUpdateReceiver);
				mBluetoothLeService.disconnect();
				mBluetoothLeService.close();
				Intent gattServiceIntent2 = new Intent(chat, RBLService.class);
				bindService(gattServiceIntent2, mServiceConnection, BIND_AUTO_CREATE);
				
			}
		});
		
		btn.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
			
			}
		});

		Intent intent = getIntent();

		mDeviceAddress = intent.getStringExtra(Device.EXTRA_DEVICE_ADDRESS);
		mDeviceName = intent.getStringExtra(Device.EXTRA_DEVICE_NAME);

		getActionBar().setTitle(mDeviceName);
		getActionBar().setDisplayHomeAsUpEnabled(false);

		Intent gattServiceIntent = new Intent(this, RBLService.class);
		bindService(gattServiceIntent, mServiceConnection, BIND_AUTO_CREATE);
	}

	@Override
	protected void onResume() {
		super.onResume();

		registerReceiver(mGattUpdateReceiver, makeGattUpdateIntentFilter());
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		if (item.getItemId() == android.R.id.home) {
			mBluetoothLeService.disconnect();
			mBluetoothLeService.close();

			System.exit(0);
		}

		return super.onOptionsItemSelected(item);
	}

	@Override
	protected void onStop() {
		super.onStop();

		unregisterReceiver(mGattUpdateReceiver);
	}

	@Override
	protected void onDestroy() {
		super.onDestroy();

		mBluetoothLeService.disconnect();
		mBluetoothLeService.close();

		System.exit(0);
	}

	private void displayData(byte[] byteArray) {
		
		if (byteArray != null) {
			String data = new String(byteArray);
			Log.d(TAG, data);
			label.setText(data);
			
		}
	}

	private void getGattService(BluetoothGattService gattService) {
		if (gattService == null)
			return;

		BluetoothGattCharacteristic characteristic = gattService
				.getCharacteristic(RBLService.UUID_BLE_SHIELD_TX);
		map.put(characteristic.getUuid(), characteristic);

		BluetoothGattCharacteristic characteristicRx = gattService
				.getCharacteristic(RBLService.UUID_BLE_SHIELD_RX);
		mBluetoothLeService.setCharacteristicNotification(characteristicRx,
				true);
		mBluetoothLeService.readCharacteristic(characteristicRx);
	}

	private static IntentFilter makeGattUpdateIntentFilter() {
		final IntentFilter intentFilter = new IntentFilter();

		intentFilter.addAction(RBLService.ACTION_GATT_CONNECTED);
		intentFilter.addAction(RBLService.ACTION_GATT_DISCONNECTED);
		intentFilter.addAction(RBLService.ACTION_GATT_SERVICES_DISCOVERED);
		intentFilter.addAction(RBLService.ACTION_DATA_AVAILABLE);

		return intentFilter;
	}
}
