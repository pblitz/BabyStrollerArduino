package de.blitzit.babystroller;

import java.util.ArrayList;
import java.util.List;

import android.app.Activity;
import android.app.Dialog;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.bluetooth.BluetoothManager;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.view.Window;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import com.redbear.chat.Device;

public class MainActivity extends Activity {
	private static final String ARDUINO_NAME = "Stroller";
	private BluetoothAdapter mBluetoothAdapter;
	private static final int REQUEST_ENABLE_BT = 1;
	private static final long SCAN_PERIOD = 3000;
	private Dialog mDialog;
	public static List<BluetoothDevice> mDevices = new ArrayList<BluetoothDevice>();
	public static MainActivity instance = null;
	
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		requestWindowFeature(Window.FEATURE_INDETERMINATE_PROGRESS);
		setContentView(R.layout.activity_main);
		if (!getPackageManager().hasSystemFeature(
				PackageManager.FEATURE_BLUETOOTH_LE)) {
			Toast.makeText(this, "Ble not supported", Toast.LENGTH_SHORT)
					.show();
			finish();
		}
		final BluetoothManager mBluetoothManager = (BluetoothManager) getSystemService(Context.BLUETOOTH_SERVICE);
		mBluetoothAdapter = mBluetoothManager.getAdapter();
		if (mBluetoothAdapter == null) {
			Toast.makeText(this, "Ble not supported", Toast.LENGTH_SHORT)
					.show();
			finish();
			return;
		}

		if (!mBluetoothAdapter.isEnabled()) {
			Intent enableBtIntent = new Intent(
					BluetoothAdapter.ACTION_REQUEST_ENABLE);
			startActivityForResult(enableBtIntent, REQUEST_ENABLE_BT);
		}
		
		
	}
	
	@Override
	protected void onRestart() {
		super.onRestart();
//		mBluetoothAdapter.
		mDevices = new ArrayList<BluetoothDevice>();
	}
	
	@Override
	protected void onStart() {
		super.onStart();
		TextView statusIndicator=(TextView)findViewById(R.id.status);
		statusIndicator.setVisibility(1);
		statusIndicator.setText("Scanning...");
		scanLeDevice();
	}
	
	class ClickListener implements OnClickListener {

		@Override
		public void onClick(View view) {
			if (view.getId()==R.id.rescanButton) {
				scanLeDevice();
				findViewById(R.id.rescanButton).setVisibility(View.INVISIBLE);
			} else {
				System.err.println("Onclick for the wrong button?!?");
			}
		}

		
		
	}
	private void scanLeDevice() {
		findViewById(R.id.status).setVisibility(View.VISIBLE);
		new Thread() {

			@Override
			public void run() {
				mBluetoothAdapter.startLeScan(mLeScanCallback);

				try {
					Thread.sleep(SCAN_PERIOD);
				} catch (InterruptedException e) {
					e.printStackTrace();
				}

				mBluetoothAdapter.stopLeScan(mLeScanCallback);
				// check if we found a correct device
				boolean found = false;
				
				for (BluetoothDevice device : mDevices) {
					if (ARDUINO_NAME.equalsIgnoreCase(device.getName())) {
						// we found it! 
						found=true;
						startNextActvitiy(device);
					    
					}
					break;
				}
				if (!found) {
				runOnUiThread(new Runnable() {
					
					@Override
					public void run() {
						findViewById(R.id.status).setVisibility(View.INVISIBLE);
						Button button = (Button) findViewById(R.id.rescanButton);
						button.setOnClickListener(new ClickListener());
						button.setVisibility(View.VISIBLE);
						
					}
				});
				
					
				} 
			}
		}.start();
	}

	private void startNextActvitiy(BluetoothDevice device) {
		Intent intent = new Intent(this, Chat.class);
		intent.putExtra(Device.EXTRA_DEVICE_ADDRESS, device.getAddress());
		intent.putExtra(Device.EXTRA_DEVICE_NAME, device.getName());
		startActivity(intent);	
	}
	
	
	private BluetoothAdapter.LeScanCallback mLeScanCallback = new BluetoothAdapter.LeScanCallback() {

		@Override
		public void onLeScan(final BluetoothDevice device, final int rssi,
				byte[] scanRecord) {
			runOnUiThread(new Runnable() {
				@Override
				public void run() {
					if (device != null) {
						if (mDevices.indexOf(device) == -1)
							mDevices.add(device);
					}
				}
			});
		}
	};

	
	@Override
	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
		// User chose not to enable Bluetooth.
		if (requestCode == REQUEST_ENABLE_BT
				&& resultCode == Activity.RESULT_CANCELED) {
			finish();
			return;
		}
		super.onActivityResult(requestCode, resultCode, data);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}

	@Override
	public boolean onOptionsItemSelected(MenuItem item) {
		// Handle action bar item clicks here. The action bar will
		// automatically handle clicks on the Home/Up button, so long
		// as you specify a parent activity in AndroidManifest.xml.
		int id = item.getItemId();
		if (id == R.id.action_settings) {
			return true;
		}
		return super.onOptionsItemSelected(item);
	}
}
