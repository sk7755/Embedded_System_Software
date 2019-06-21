package com.example.androidex;

import com.example.androidex.MyService.Counter;

import android.app.Activity;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.graphics.Color;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.util.Log;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.LinearLayout;
import android.widget.TextView;

public class MainActivity2 extends Activity{
		//@Override
		LinearLayout linear;
		EditText data;
		Button make_btn;
		OnClickListener ltn;
		int row = 0, col = 0;
		int black_row, black_col;
		int dir[][] = {{1,0},{0,1},{-1,0},{0,-1}};
		int current_sec = 0;
		public MyService mService;
		public TextView tv;
		public boolean isBind;
		Thread viewtimer;
		public Handler handler = new Handler(){
			@Override
			public void handleMessage(Message msg){
				if(msg.what == 0){
					
					tv.setText(String.format("%02d",current_sec/60) + ":" + String.format("%02d",current_sec%60));
				}
			}
		};
		
	protected boolean check_answer(){
		for(int i = 0 ; i< row;i++){
			for(int j = 0;j<col; j++){
				Button cur = (Button)findViewById(i * col + j);
				if (!Integer.toString(cur.getId() + 1).equals(cur.getText().toString()))
					return false;
			}
		}
		return true;
	}
	
	protected void swap_button(int i, int j){
		if(i < 0 || i>= row || j < 0 || j>=col)
			return;
		int tmp = Math.abs(i - black_row) + Math.abs(j - black_col);
		
		if(tmp != 1)
			return;
		Button black_btn = (Button)findViewById(black_row * col + black_col);
		Button swap_btn = (Button)findViewById(i * col + j);
		
		black_btn.setBackgroundResource(android.R.drawable.btn_default);
		CharSequence tmp_text = black_btn.getText();
		black_btn.setText(swap_btn.getText());
		swap_btn.setText(tmp_text);
		swap_btn.setBackgroundColor(Color.BLACK);
		black_row = i;
		black_col = j;
	}
	
	protected void shuffle(){
		for(int k = 0 ; k< row*col*10; k++){
			int tmp = (int)(Math.random() * 4);
			
			swap_button(black_row + dir[tmp][0] ,black_col + dir[tmp][1]);
		}
	}
	
	protected void MakeButtons(){
		for(int i = 0 ; i < row; i++){
			LinearLayout.LayoutParams params = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.MATCH_PARENT,row);
			LinearLayout puzzle_linear = new LinearLayout(this);
			puzzle_linear.setOrientation(LinearLayout.HORIZONTAL);
			puzzle_linear.setLayoutParams(params);
			params = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.MATCH_PARENT,1);
		
			for(int j = 0 ; j < col; j++){
				Button puzzle_btn = new Button(this);
				puzzle_btn.setText(Integer.toString(i*col + j +1));
				puzzle_btn.setLayoutParams(params);
				puzzle_btn.setId(i*col + j);
				ltn=new OnClickListener(){
					public void onClick(View v){
						swap_button(v.getId()/col,v.getId() % col);
						if(check_answer()){
							finish();
						}
					}
				};
				puzzle_btn.setOnClickListener(ltn);
				puzzle_linear.addView(puzzle_btn);
			}
			linear.addView(puzzle_linear);
		}
		Button tmp = (Button) findViewById(row*col -1);
		tmp.setBackgroundColor(Color.BLACK);
		
	}
	
	public class ViewTimer implements Runnable{
		
		@Override
	    	public void run() {
				int pre_sec = 0;
				int tmp =pre_sec;
	    		while(!MyService.isStop)
				{
	    			if(tmp > pre_sec){
	    				Log.e(Integer.toString(tmp), "view timer");
	    				handler.sendEmptyMessage(0);
	    				pre_sec = tmp;
	    				current_sec++;
	    			}
	    			// TODO Auto-generated method stub
	    			try {
	    				Thread.sleep(100);
	    			} catch (InterruptedException e) {
	    				// TODO Auto-generated catch block
	    				e.printStackTrace();
	    			}
	    			tmp = mService.get_time();
	    		}
	    	}
	}
	
	ServiceConnection sconn = new ServiceConnection(){
        @Override //서비스가 실행될 때 호출
        public void onServiceConnected(ComponentName name, IBinder service) {
            MyService.MyBinder myBinder = (MyService.MyBinder) service;
            mService = myBinder.getService();
            isBind = true;
            Log.e("LOG", "onServiceConnected()");
        }

        @Override //서비스가 종료될 때 호출
        public void onServiceDisconnected(ComponentName name) {
            mService = null;
            isBind = false;
            Log.e("LOG", "onServiceDisconnected()");
        }
	};
	
	protected void onCreate(Bundle savedInstanceState) {
		// TODO Auto-generated method stub
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_main2);
		
		linear = (LinearLayout)findViewById(R.id.container);
		tv = (TextView)findViewById(R.id.timerView);
		data=(EditText)findViewById(R.id.editText1);
		make_btn=(Button)findViewById(R.id.makebutton);
		
		
		ltn=new OnClickListener(){
			public void onClick(View v){
				String temp=data.getText().toString();
				if(row * col > 0){
					
					linear.removeViewsInLayout(3 ,row);
				}
				row = Integer.parseInt(temp.split(" ")[0]);
				col = Integer.parseInt(temp.split(" ")[1]);
				black_row = row-1;
				black_col = col-1;
				MakeButtons();

				
				while(check_answer()){
					shuffle();
				}
				
				 Intent intent = new Intent(
						 MainActivity2.this, // 현재 화면
						 MyService.class); // 다음넘어갈 컴퍼넌트

				bindService(intent, // intent 객체
						 sconn, // 서비스와 연결에 대한 정의
						 Context.BIND_AUTO_CREATE);
				current_sec =0 ;
				MyService.isStop = false;
				if(viewtimer == null){    
					viewtimer = new Thread(new ViewTimer());
					viewtimer.start();
				}
			};
		};
		make_btn.setOnClickListener(ltn);
	}
}



