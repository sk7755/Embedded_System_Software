package com.example.androidex;

import android.app.Service;
import android.content.Intent;
import android.os.Binder;
import android.os.Handler;
import android.os.IBinder;
import android.util.Log;


//서비스 클래스를 구현하려면, Service 를 상속받는다
public class MyService extends Service {
	public static boolean isStop = false;
	public int count = 0;
    private IBinder mIBinder = new MyBinder();
    Thread counter = null;
    class MyBinder extends Binder{
        MyService getService(){
            return MyService.this;
        }
    }
    @Override
    public IBinder onBind(Intent intent) {
        Log.e("LOG", "onBind()");
        
        return mIBinder;
    }

    @Override
    public void onCreate() {
        Log.e("LOG", "onCreate()");
        super.onCreate();
        count = 0;
        counter = new Thread(new Counter());
        counter.start();
    }

    @Override
    public int onStartCommand(Intent intent, int flags, int startId) {
        Log.e("LOG", "onStartCommand()");
        return super.onStartCommand(intent, flags, startId);
    }

    @Override
    public void onDestroy() {
        Log.e("LOG", "onDestroy()");
        super.onDestroy();
    }

    public class Counter implements Runnable{
    	public Handler handler = new Handler();
    	@Override
    	public void run() {
    		
    		while(!MyService.isStop){
    			// TODO Auto-generated method stub
    			try {
    				Thread.sleep(1000);
    			} catch (InterruptedException e) {
    				// TODO Auto-generated catch block
    				e.printStackTrace();
    			}
    			Log.e(Integer.toString(count), "counter");
    			count++;
    			
    		}
    	}
    }
    
    public int get_time(){
    	return count;
    }
    
    @Override
    public boolean onUnbind(Intent intent) {
        Log.e("LOG", "onUnbind()");
        isStop = true;
        return super.onUnbind(intent);
    }
}

