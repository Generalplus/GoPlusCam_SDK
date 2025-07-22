package generalplus.com.GPCamDemo;

import android.Manifest;
import android.annotation.TargetApi;
import android.app.Activity;
import android.app.AlertDialog;
import android.app.ProgressDialog;
import android.content.Context;
import android.content.ContextWrapper;
import android.content.DialogInterface;
import android.content.Intent;
import android.content.SharedPreferences;
import android.content.pm.PackageManager;
import android.net.ConnectivityManager;
import android.net.Network;
import android.net.NetworkCapabilities;
import android.net.NetworkRequest;
import android.net.Uri;
import android.net.wifi.WifiInfo;
import android.net.wifi.WifiManager;
import android.os.Build;
import android.os.Bundle;
import android.os.Environment;
import android.os.Handler;
import android.text.InputType;
import android.util.Log;
import android.view.View;
import android.view.WindowManager;
import android.widget.EditText;
import android.widget.ImageButton;
import android.widget.TextView;
import android.widget.Toast;

import java.io.BufferedReader;
import java.io.File;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStreamReader;
import java.math.BigInteger;
import java.nio.ByteOrder;
import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Calendar;
import java.util.List;
import java.util.Locale;
import java.util.concurrent.TimeoutException;

import androidx.core.app.ActivityCompat;
import generalplus.com.GPCamLib.CamWrapper;
import generalplus.com.GPCamLib.GPINIReader;


public class MainActivity extends Activity {
	
	private static String TAG = "MainActivity";
	private CamWrapper	m_CamWrapper;
	private GPINIReader m_GPINIReader;
	private ProgressDialog m_Dialog;
	private String strSaveDirectory;
	private Context mContext;
	private ImageButton imgbtn_connect;
	private int m_iSetTime = 0;
	private int m_inputSelection = 0;
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        if (getIntent().getBooleanExtra("EXIT", false)) {
            finish();
        }

        setContentView(R.layout.activity_main); 
        getWindow().addFlags(WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
        getActionBar().setDisplayShowHomeEnabled(false);
        mContext = MainActivity.this;

		if (shouldAskPermission()) {
			int writePermission = ActivityCompat.checkSelfPermission(this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

			if (writePermission != PackageManager.PERMISSION_GRANTED) {
				// We don't have permission so prompt the user
				String[] perms = {"android.permission.WRITE_EXTERNAL_STORAGE"};

				int permsRequestCode = 200;

				ActivityCompat.requestPermissions(this, perms, permsRequestCode);
			}
			else {
				crateDirectory();
			}
		}
		else {
			crateDirectory();
		}

		if(m_CamWrapper == null)
		{
			m_CamWrapper = new CamWrapper();
		}

		if(m_GPINIReader == null)
			m_GPINIReader = new GPINIReader(getExternalFilesDirs(CamWrapper.CamDefaulFolderName)[0].getAbsolutePath() + "/" + CamWrapper.ConfigFileName);
		
		imgbtn_connect = (ImageButton) findViewById(R.id.imgbtn_connect); 
		imgbtn_connect.setOnClickListener(new View.OnClickListener(){

			@Override
			public void onClick(View v) {
				if (shouldAskPermission()) {
					int writePermission = ActivityCompat.checkSelfPermission(MainActivity.this, Manifest.permission.WRITE_EXTERNAL_STORAGE);

					if (writePermission != PackageManager.PERMISSION_GRANTED) {

						AlertDialog.Builder editDialog = new AlertDialog.Builder(MainActivity.this);
						editDialog.setTitle(getResources().getString(R.string.app_name));
						editDialog.setMessage(getResources().getString(R.string.Permission_denied));

						editDialog.setPositiveButton("OK", new DialogInterface.OnClickListener() {
							// do something when the button is clicked
							public void onClick(DialogInterface arg0, int arg1) {

							}
						});
						editDialog.show();
						return;
					}
				}


				if(m_connectGPWifiDeviceThread == null)
            	{
            		if (m_Dialog == null) {
        				m_Dialog = new ProgressDialog(mContext);
        				m_Dialog.setCanceledOnTouchOutside(false);
        				m_Dialog.setMessage(getResources().getString(R.string.Connecting));
        			}
        			m_Dialog.show();
            		
            		m_connectGPWifiDeviceThread = new Thread(new ConnectGPWifiDeviceRunnable());
            		m_connectGPWifiDeviceThread.start();
            	}				
			}
			
		});

		SharedPreferences preferences = getSharedPreferences("Data", 0);
		m_iSetTime = preferences.getInt("SetTime",0);

		if(isExternalStorageWritable())
			recordLogCatHandler.postDelayed(runnableRecordLogCat, 0);
		TextView tv_version = (TextView) findViewById(R.id.tv_version);
		tv_version.setOnClickListener(new View.OnClickListener(){

			@Override
			public void onClick(View v) {
				List<String> lunch = new ArrayList<String>();
				lunch.add(getResources().getString(R.string.Set_Wifi_sport_Cam_Time));
				lunch.add(getResources().getString(R.string.Save_Log));
				lunch.add(getResources().getString(R.string.Set_Buffering_Time));
				new AlertDialog.Builder(MainActivity.this)
						.setTitle(getResources().getString(R.string.Setting))
						.setItems(lunch.toArray(new String[lunch.size()]), new DialogInterface.OnClickListener() {
							@Override
							public void onClick(DialogInterface dialog, int which) {
								if (0 == which) {
									showSetTimeDialog();
								}
								else if (1 == which) {
									showSaveLogDialog();
								}
								else {
									showSetBufferingTimeDialog();
								}
								dialog.dismiss();
							}
						})
						.show();
			}

		});
		Log.e(TAG,"Version = " + getResources().getString(R.string.app_version));

		if (Build.VERSION.SDK_INT >= 21) {
			NetworkRequest.Builder builder = new NetworkRequest.Builder();
			// 设置指定的网络传输类型(蜂窝传输) 等于手机网络
			builder.addTransportType(NetworkCapabilities.TRANSPORT_WIFI);

			final ConnectivityManager connectivityManager = (ConnectivityManager) this.getSystemService(Context.CONNECTIVITY_SERVICE);
			NetworkRequest request = builder.build();
			ConnectivityManager.NetworkCallback callback = new ConnectivityManager.NetworkCallback() {
				@TargetApi(Build.VERSION_CODES.M)
				@Override
				public void onAvailable(Network network) {
					super.onAvailable(network);
					Log.i("test", "已根据功能和传输类型找到合适的网络");
					// 可以通过下面代码将app接下来的请求都绑定到这个网络下请求
					if (Build.VERSION.SDK_INT >= 23) {
						connectivityManager.bindProcessToNetwork(network);
					} else {// 23后这个方法舍弃了
						ConnectivityManager.setProcessDefaultNetwork(network);
					}
					// 也可以在将来某个时间取消这个绑定网络的设置
					// if (Build.VERSION.SDK_INT >= 23) {
					//      onnectivityManager.bindProcessToNetwork(null);
					//} else {
					//     ConnectivityManager.setProcessDefaultNetwork(null);
				}
			};
			//connMgr.requestNetwork(request, callback);
			connectivityManager.registerNetworkCallback(request, callback);
		}


    }

	private void showSetTimeDialog() {
		final CharSequence[] items = {getResources().getString(R.string.Disable), getResources().getString(R.string.Enable)};

		// Creating and Building the Dialog
		AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
		builder.setTitle(getResources().getString(R.string.Set_Wifi_sport_Cam_Time));

		builder.setSingleChoiceItems(items, m_iSetTime,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int item) {
						if (m_iSetTime != item) {
							SharedPreferences preferences = getSharedPreferences("Data", 0);
							SharedPreferences.Editor editor = preferences.edit();
							editor.putInt("SetTime",item);
							editor.commit();
							m_iSetTime = item;
						}

						dialog.dismiss();
					}
				});
		builder.show();
	}

	private void crateDirectory() {
		File[] SaveFileDirectory = getExternalFilesDirs(CamWrapper.CamDefaulFolderName);

		strSaveDirectory = SaveFileDirectory[0].getAbsolutePath();

		if (!SaveFileDirectory[0].exists()) {
			boolean abc = SaveFileDirectory[0].mkdirs();
		}

		File CameraFileDirectory = new File(Environment.getExternalStorageDirectory().getPath() + CamWrapper.SaveFileToDevicePath);
		if(!CameraFileDirectory.exists())
			CameraFileDirectory.mkdirs();
	}

	@Override
	public void onRequestPermissionsResult(int permsRequestCode, String[] permissions, int[] grantResults){
		if(null == grantResults) {
			return;
		}

		switch(permsRequestCode){

			case 200:

				if (grantResults.length < 1) {
					return;
				}
				boolean writeAccepted = grantResults[0]== PackageManager.PERMISSION_GRANTED;
				if (false == writeAccepted) {
					/*if (shouldAskPermission()) {
						String[] perms = {"android.permission.WRITE_EXTERNAL_STORAGE"};

						ActivityCompat.requestPermissions(this, perms, permsRequestCode);
					}*/
				}
				else {
					crateDirectory();
				}
				break;
		}
	}

	private boolean shouldAskPermission(){

		return(Build.VERSION.SDK_INT>Build.VERSION_CODES.LOLLIPOP_MR1 && Build.VERSION.SDK_INT< Build.VERSION_CODES.TIRAMISU);

	}

	private void showSetBufferingTimeDialog() {
		AlertDialog.Builder editDialog = new AlertDialog.Builder(this);
		editDialog.setTitle(getResources().getString(R.string.Set_Buffering_Time));

		SharedPreferences preferences = getSharedPreferences("Data", 0);
		int iSetBufferingTime = preferences.getInt("SetBufferingTime",40);

		final EditText editText = new EditText(this);
		editText.setInputType(InputType.TYPE_CLASS_NUMBER);
		editText.setText(iSetBufferingTime + "");
		editDialog.setView(editText);

		editDialog.setPositiveButton("OK", new DialogInterface.OnClickListener() {
			// do something when the button is clicked
			public void onClick(DialogInterface arg0, int arg1) {

				try {
					String strTemp = editText.getText().toString();
					int iTemp = Integer.parseInt(strTemp);


					SharedPreferences preferences = getSharedPreferences("Data", 0);
					SharedPreferences.Editor editor = preferences.edit();
					editor.putInt("SetBufferingTime",iTemp);
					editor.commit();
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}
		});
		editDialog.setNegativeButton("Cancel", new DialogInterface.OnClickListener() {
			// do something when the button is clicked
			public void onClick(DialogInterface arg0, int arg1) {

			}
		});
		editDialog.show();
	}

	private void showSaveLogDialog() {
		final CharSequence[] items = {getResources().getString(R.string.Disable), getResources().getString(R.string.Enable)};

		// Creating and Building the Dialog
		AlertDialog.Builder builder = new AlertDialog.Builder(MainActivity.this);
		builder.setTitle(getResources().getString(R.string.Save_Log));

		builder.setSingleChoiceItems(items, m_inputSelection,
				new DialogInterface.OnClickListener() {
					public void onClick(DialogInterface dialog, int item) {
						m_inputSelection = item;

						recordLogCatHandler.removeCallbacks(runnableRecordLogCat);
						bWriteLogCatFile = false;
						if (1 == item) {
							if (isExternalStorageWritable()) {
								bWriteLogCatFile = true;
								recordLogCatHandler.postDelayed(runnableRecordLogCat, 0);
							}
						}
						dialog.dismiss();
					}
				});
		builder.show();
	}

	private static class Worker extends Thread {
		private final Process process;
		private Integer exit;

		private Worker(Process process) {
			this.process = process;
		}

		public void run() {
			try {
				exit = process.waitFor();
			} catch (InterruptedException ignore) {
                return;
            }
        }
    }

    public static int executeCommandLine(final String commandLine,
            final long timeout)
            throws IOException, InterruptedException, TimeoutException
    {
		Runtime runtime = Runtime.getRuntime();
		Process process = runtime.exec(commandLine);
		
		Worker worker = new Worker(process);
		worker.start();
		try {
		worker.join(timeout);
		if (worker.exit != null)
		return worker.exit;
		else
		throw new TimeoutException();
		} catch(InterruptedException ex) {
		worker.interrupt();
		Thread.currentThread().interrupt();
		throw ex;
		} finally {
		process.destroy();
		}
	}

    
    private static Thread m_connectGPWifiDeviceThread = null;
	class ConnectGPWifiDeviceRunnable implements Runnable{
		
		//Check Wifi Status
		private boolean bCheckWifiStatus = false;
		private int i32RetryCheckWifiStatusCount = 100;
		private int i32CheckWifiStatusDelayTime = 200;
		
		//Check Connect Status;
		private boolean bCheckConnectStatus = false;
		private int i32Status;
		
		private int i32RetryCount = 20;

		@Override
		public void run() {
			// TODO Auto-generated method stub			
            if (true == bCheckConnectStatus) {
                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    // TODO Auto-generated catch block
                    e.printStackTrace();
                }
            }

			WifiManager wifiManager = (WifiManager) mContext.getSystemService(WIFI_SERVICE);
			WifiInfo wifiInfo = wifiManager.getConnectionInfo();
			if (wifiInfo != null) {
				int ipAddress = wifiManager.getConnectionInfo().getIpAddress();
				if (0 == ipAddress) {
					bCheckWifiStatus = false;
				} else {
					try {
						// Convert little-endian to big-endianif needed
						if (ByteOrder.nativeOrder().equals(ByteOrder.LITTLE_ENDIAN)) {
							ipAddress = Integer.reverseBytes(ipAddress);
						}
						byte[] ipByteArray = BigInteger.valueOf(ipAddress).toByteArray();
						CamWrapper.COMMAND_URL = (ipByteArray[0] & 0xFF) + "." + (ipByteArray[1] & 0xFF) + "."
								+ (ipByteArray[2] & 0xFF) + ".1";
						bCheckWifiStatus = true;
					} catch (Exception e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
				}
			}

			if(bCheckWifiStatus)
			{
				try {
					CamWrapper.getComWrapperInstance().GPCamConnectToDevice(CamWrapper.COMMAND_URL, CamWrapper.COMMAN_PORT);
				} catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}

				while(bCheckWifiStatus)
				{
					try {
						Thread.sleep(500);
					} catch (InterruptedException e) {
						// TODO Auto-generated catch block
						e.printStackTrace();
					}
					i32Status = CamWrapper.getComWrapperInstance().GPCamGetStatus();
					switch(i32Status)
					{
					case CamWrapper.GPTYPE_ConnectionStatus_Idle:
						bCheckConnectStatus = false;
						bCheckWifiStatus = true;
						break;
					case CamWrapper.GPTYPE_ConnectionStatus_Connecting:
						bCheckConnectStatus = false;
						bCheckWifiStatus = true;
						break;
					case CamWrapper.GPTYPE_ConnectionStatus_Connected:
						bCheckConnectStatus = true;
						bCheckWifiStatus = false;
						CamWrapper.getComWrapperInstance().SetGPCamSetDownloadPath(strSaveDirectory);
						//CamWrapper.getComWrapperInstance().SetGPCamSendGetParameterFile(CamWrapper.ParameterFileName);
						CamWrapper.getComWrapperInstance().GPCamGetStatus();						
						break;
					case CamWrapper.GPTYPE_ConnectionStatus_DisConnected:
						i32RetryCount--;
						if(i32RetryCount == 0)
						{
							bCheckConnectStatus = false;
							bCheckWifiStatus = false;
							CamWrapper.getComWrapperInstance().GPCamDisconnect();
							break;
						}
						break;
					case CamWrapper.GPTYPE_ConnectionStatus_SocketClosed:
						i32RetryCount--;
						if(i32RetryCount == 0)
						{
							bCheckConnectStatus = false;
							bCheckWifiStatus = false;
							CamWrapper.getComWrapperInstance().GPCamDisconnect();
							break;
						}
						break;
					}
				}
			}

			try {
				MainActivity.this.runOnUiThread(new Runnable(){

					@Override
					public void run() {
						if (m_Dialog != null) {
							if (m_Dialog.isShowing()) {
								m_Dialog.dismiss();
								m_Dialog = null;
							}
						}
						if(bCheckConnectStatus)
						{
							Intent toVlcPlayer = new Intent(MainActivity.this, MainViewController.class);
							Bundle b = new Bundle();
							b.putInt("SetTime", m_iSetTime);
							toVlcPlayer.putExtras(b);
							startActivity(toVlcPlayer);
							//finish();
						}
						else
						{
							Toast.makeText(mContext,  getResources().getString(R.string.Please_connect_message), Toast.LENGTH_SHORT).show();
							try {
								Intent intent = new Intent(WifiManager.ACTION_PICK_WIFI_NETWORK);
								startActivity(intent);
							} catch (Exception e) {
							}
						}
						m_connectGPWifiDeviceThread = null;
					}

				});
			}catch (Exception e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
			}
		}
		
	}

	@Override
	protected void onDestroy() {
		Log.e(TAG, "onDestroy ...");
		if (m_Dialog != null) {
			if (m_Dialog.isShowing()) {
				m_Dialog.dismiss();
				m_Dialog = null;
			}
		}
		recordLogCatHandler.removeCallbacks(runnableRecordLogCat);
		super.onDestroy();
	}

	private boolean bWriteLogCatFile = false;
	private String strFileName = "";
	// Thread for recording LOGCAT
	int MAX_LOGCAT_TIMES = 150;
	int i32RecordLogCatCounter = 0;
	Handler recordLogCatHandler = new Handler();
	Runnable runnableRecordLogCat = new Runnable() {

		@Override
		public void run() {

			if(bWriteLogCatFile)
			{
				// Save to another file
				i32RecordLogCatCounter++;
				if(i32RecordLogCatCounter >= MAX_LOGCAT_TIMES)
				{
					strFileName = "";
					i32RecordLogCatCounter = 0;
				}

				// Write LogCat to file
//				try {
//					strFileName = GetFileName();
//					//Process process = Runtime.getRuntime().exec("logcat -c");
//					//Runtime.getRuntime().exec("logcat -c");
//					Runtime.getRuntime().exec("logcat -v time -f " + strFileName);
//
//					//Toast.makeText(getApplicationContext(), "Save File Sucess", Toast.LENGTH_SHORT).show();
//
//				} catch (IOException e) {
//
//					strFileName = "";
//					Toast.makeText(getApplicationContext(), "Save File Fail", Toast.LENGTH_SHORT).show();
//					e.printStackTrace();
//				}

				writeLog();
				recordLogCatHandler.postDelayed(this, 2000);
			}
			else
			{
				recordLogCatHandler.removeCallbacks(runnableRecordLogCat);
			}

		}
	};

	private FileOutputStream fos;
	private void writeLog() {

		try{
                            /*命令的准备*/
			ArrayList<String> getLog = new ArrayList<String>();
			getLog.add("logcat");
			getLog.add("-d");
			getLog.add("-v");
			getLog.add("time");
			ArrayList<String> clearLog = new ArrayList<String>();
			clearLog.add("logcat");
			clearLog.add("-c");

			Process process = Runtime.getRuntime().exec(getLog.toArray(new String[getLog.size()]));//抓取当前的缓存日志

			BufferedReader buffRead = new BufferedReader(new InputStreamReader(process.getInputStream()));//获取输入流
			//Runtime.getRuntime().exec(clearLog.toArray(new String[clearLog.size()]));//清除是为了下次抓取不会从头抓取
			String str = null;

			strFileName = GetFileName();
			File logFile = new File(strFileName);//打开文件
			fos = new FileOutputStream(logFile,false);//true表示在写的时候在文件末尾追加
			String newline = System.getProperty("line.separator");//换行的字符串
			//Date date = new Date(System.currentTimeMillis());
			//String time = format.format(date);


			//Log.i(TAG, "thread");
			while((str=buffRead.readLine())!=null){//循环读取每一行
				//Runtime.getRuntime().exec(clearLog.toArray(new String[clearLog.size()]));
				//Log.i(TAG, str);
				fos.write((str).getBytes());//加上年
				fos.write(newline.getBytes());//换行
			}
			fos.close();
			fos = null;
			//Runtime.getRuntime().exec(clearLog.toArray(new String[clearLog.size()]));
		}catch(Exception e){
			strFileName = "";
			Toast.makeText(getApplicationContext(), "Save File Fail", Toast.LENGTH_SHORT).show();
			e.printStackTrace();
		}
	}

	private String GetFileName()
	{
		if(0 == strFileName.compareToIgnoreCase(""))
		{
			// Get current time
			SimpleDateFormat df = new SimpleDateFormat("yyyy MM dd HH mm ss", Locale.CHINESE);
			Calendar c = Calendar.getInstance();
			String strTimeStamp = df.format(c.getTime());
			String TrimTimeStamp = strTimeStamp.replace(" ", "");

			String strYear			= TrimTimeStamp.substring(0, 4);
			String strMonth			= TrimTimeStamp.substring(4, 6);
			String strDay			= TrimTimeStamp.substring(6, 8);
			String strHour			= TrimTimeStamp.substring(8, 10);
			String strMinute		= TrimTimeStamp.substring(10, 12);
			String strSecond		= TrimTimeStamp.substring(12, 14);

			boolean bSuccess = false;
			File documentsDirectory = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS) + "");
			if ( !documentsDirectory.exists() ) {		// create documents folder
				bSuccess = documentsDirectory.mkdir();
			}

			File logDirectory = new File(Environment.getExternalStoragePublicDirectory(Environment.DIRECTORY_DOCUMENTS) + "/LOGCAT");
			if ( !logDirectory.exists() ) {		// create log folder
				bSuccess = logDirectory.mkdir();
			}

			strFileName = logDirectory.getAbsolutePath() + "/LOGCAT_" + strYear + "_" + strMonth + "_" + strDay + "_" + strHour + "_"
					+ strMinute + "_" + strSecond + "_" + "Log.txt";

			// Rescan File
			File dir = new File(strFileName);
			sendBroadcast(new Intent(Intent.ACTION_MEDIA_SCANNER_SCAN_FILE,
					Uri.parse("file://" + dir.getAbsolutePath())));
		}

		return strFileName;
	}

	/* Checks if external storage is available for read and write */
	public boolean isExternalStorageWritable() {
		String state = Environment.getExternalStorageState();
		if ( Environment.MEDIA_MOUNTED.equals( state ) ) {
			return true;
		}
		return false;
	}
}
