package de.blitzit.babystroller;

import java.text.DecimalFormat;
import java.util.HashMap;
import java.util.Map;
import java.util.UUID;
import java.util.regex.Matcher;
import java.util.regex.Pattern;

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
import android.os.Handler;
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
	private TextView result;
	private String currentLine;
	private TextView rawResult;
	private RBLService mBluetoothLeService;
	private Map<UUID, BluetoothGattCharacteristic> map = new HashMap<UUID, BluetoothGattCharacteristic>();

	private ServiceConnection mServiceConnection;

	private ServiceConnection createServiceConnection() {
		ServiceConnection connection = new ServiceConnection() {

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
		return connection;
	}

	private final BroadcastReceiver mGattUpdateReceiver = new BroadcastReceiver() {
		@Override
		public void onReceive(Context context, Intent intent) {
			final String action = intent.getAction();

			if (RBLService.ACTION_GATT_DISCONNECTED.equals(action)) {

			} else if (RBLService.ACTION_GATT_SERVICES_DISCOVERED
					.equals(action)) {
				getGattService(mBluetoothLeService.getSupportedGattService());
				Log.d(TAG, "Service discoverd!");
				new Handler().postDelayed(new Runnable() {
					
					@Override
					public void run() {
						instanciateConnection();
						
					}
				},500);
				
			} else if (RBLService.ACTION_DATA_AVAILABLE.equals(action)) {
				displayData(intent.getByteArrayExtra(RBLService.EXTRA_DATA));
			} else if (RBLService.ACTION_GATT_CONNECTED.equals(action)) {
				Log.d(TAG, "Service connected!");
				// instanciateConnection();
			}
		}
	};

	private Chat chat;

	protected void instanciateConnection() {
		BluetoothGattCharacteristic characteristic = map
				.get(RBLService.UUID_BLE_SHIELD_TX);

		String str = "r";
		byte b = 0x00;
		byte[] tmp = str.getBytes();
		byte[] tx = new byte[tmp.length + 1];
		tx[0] = b;
		for (int i = 1; i < tmp.length + 1; i++) {
			tx[i] = tmp[i - 1];
		}
		characteristic.setValue(tx);
		mBluetoothLeService.writeCharacteristic(characteristic);
		Log.d(TAG, "sending out data to instanciate connection!");
	}

	@Override
	protected void onCreate(Bundle savedInstanceState) {
		chat = this;

		super.onCreate(savedInstanceState);
		setContentView(R.layout.second);

		btn = (Button) findViewById(R.id.send);
		btnRetry = (Button) findViewById(R.id.retry);
		result = (TextView) findViewById(R.id.result);
		rawResult = (TextView) findViewById(R.id.resultRaw);
		btnRetry.setOnClickListener(new OnClickListener() {

			@Override
			public void onClick(View v) {
				try {
					if (!mBluetoothLeService.isConnected()) {
						connectToDevice();
					} else {
						instanciateConnection();
					}

				} catch (Exception e) {
					Log.d(TAG,
							"Exception trying the data connection, retyring.. ",
							e);
//					unregisterReceiver(mGattUpdateReceiver);
					mBluetoothLeService.disconnect();
					mBluetoothLeService.close();
					
					connectToDevice();
//					registerReceiver(mGattUpdateReceiver,
//							makeGattUpdateIntentFilter());
				}
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
		connectToDevice();
	}

	private void connectToDevice() {
		mServiceConnection = createServiceConnection();
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

		// System.exit(0);
	}

	private void displayData(byte[] byteArray) {

		if (byteArray != null) {
			String data = new String(byteArray);
			// Log.d(TAG, data);
			data = data.replaceAll("\0", "");

			if (data.startsWith("D:")) {
				// we have a new line, probably
				currentLine = data;
			} else {
				currentLine = currentLine + data;
			}
			rawResult.setText(currentLine);
			if (currentLine.matches(".*\n\n")) {
				// end of a line
				Log.d(TAG, "Completed a line transfer: " + currentLine);
				Pattern matchPattern = Pattern
						.compile("D:\\s*(\\d*):\\s*(\\d*)\n\n");
				Matcher tagmatch = matchPattern.matcher(currentLine);
				while (tagmatch.find()) {
					String time = tagmatch.group(1);
					String distance = tagmatch.group(2);
					Long dist = Long.parseLong(distance);// in mm for now!
					double distanceM = (dist / (long) 1000);
					DecimalFormat df2 = new DecimalFormat("#,##0.00");

					result.setText("Current Distance: " + df2.format(distanceM)
							+ "m");
					Log.i(TAG, "Parsed info into: " + time + " seconds and "
							+ distanceM + "m");
				}

				// terminating the connection after it worked !
//				unregisterReceiver(mGattUpdateReceiver);
				mBluetoothLeService.disconnect();
				mBluetoothLeService.close();
//				connected = false;
			}

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
