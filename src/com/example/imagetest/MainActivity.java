package com.example.imagetest;

import java.io.File;

import android.os.Bundle;
import android.provider.MediaStore;
import android.app.Activity;
import android.content.DialogInterface;
import android.content.DialogInterface.OnClickListener;
import android.content.Intent;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.util.Log;
import android.view.Menu;

import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import android.view.Menu;

import android.view.MotionEvent;
import android.view.View;
import android.view.Window;
import android.view.WindowManager;

import android.widget.ImageButton;
import android.widget.ImageView;
import org.opencv.core.Mat;
import org.opencv.highgui.*;
public class MainActivity extends Activity {

	
	ImageButton shutterButton;
	Bitmap mImageBitmap;
	ImageView mImageView;
	private Mat mRgba;
	private Mat disp;
	private Mat finalImage;
	private Mat limg;
	String TAG="SimpleImageCapture";
	private File imgFile;
	private File leftimgFile;
	private Bitmap myBitmap;
			
	@Override
	protected void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		
		this.requestWindowFeature(Window.FEATURE_NO_TITLE);
		this.getWindow().setFlags(WindowManager.LayoutParams.FLAG_FULLSCREEN, WindowManager.LayoutParams.FLAG_FULLSCREEN);
		
		setContentView(R.layout.activity_main);
		
		shutterButton=(ImageButton) findViewById(R.id.shutterButton);
		mImageView=(ImageView) findViewById(R.id.imageView1);
		System.loadLibrary("disp_img");
		shutterButton.setOnClickListener((android.view.View.OnClickListener) new MyOnClickListener());
		mImageView.setOnTouchListener(new TouchListener());
		    
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		// Inflate the menu; this adds items to the action bar if it is present.
		getMenuInflater().inflate(R.menu.main, menu);
		return true;
	}
	

	protected void onActivityResult(int requestCode, int resultCode, Intent data) {
	    super.onActivityResult(requestCode, resultCode, data);
	    
	    Log.d("Request code:"+requestCode,"tea");
	    
	    switch(requestCode){
	    case 1337:
	        if(resultCode==RESULT_OK)
	        {
	   
	        	// @Jay : Change this to part to full_URI
		    	imgFile = new  File((String) data.getExtras().get("full_URI"));
		    	leftimgFile = new File((String) data.getExtras().get("left_URI"));
		    	Log.d("full_URI","url="+imgFile);
		    	mRgba = new Mat();
		    	disp = new Mat();
		    	limg = new Mat();
		    	mRgba = Highgui.imread(imgFile.getAbsolutePath());
		    	Log.d(TAG, "Image loaded");
		    	getDisparity(mRgba.getNativeObjAddr(), disp.getNativeObjAddr());
		    	crop5(mRgba.getNativeObjAddr(), limg.getNativeObjAddr());
		    	Log.d(TAG, "Got Disparity");
		    	Highgui.imwrite(leftimgFile.getAbsolutePath(), limg);
		    	if(imgFile.exists())
		    	{
			    	myBitmap = BitmapFactory.decodeFile(leftimgFile.getAbsolutePath());
				    mImageView.setImageBitmap(myBitmap);
		
			    }
	        }
	    }
	}
	
	class MyOnClickListener implements View.OnClickListener{

		public void onClick(View v) {
			
			    Intent camera_intent=new Intent(getBaseContext(),CustomClickActivity.class);
			    startActivityForResult(camera_intent,1337);
		
		}
	     };
	     class TouchListener implements View.OnTouchListener{

	 		@Override
	 		public boolean onTouch(View v, MotionEvent event) {

	 			if(event.getAction() == MotionEvent.ACTION_DOWN) 
	 			{
	 				Log.d(TAG,"X ="+(event.getRawX()-mImageView.getLeft())+"  Y= "+(event.getRawY()-mImageView.getTop())); // For landscape orientation,i.e max val of x is 800 and y max value is 480 ..

	 				// Pass these to the JNI function
	 				float converted_xcoord=(event.getRawX()-mImageView.getLeft());
	 				float converted_ycoord=(event.getRawY()-mImageView.getTop());
	 				converted_xcoord=(converted_xcoord/mImageView.getWidth())*500;
	 				converted_ycoord=(converted_ycoord/mImageView.getHeight())*500;
	 				Log.d(TAG, String.valueOf(converted_ycoord));
	 				Log.d(TAG, String.valueOf(converted_xcoord));
	 				Log.d(TAG, "converted");
			    	finalImage = new Mat();
			    	getThreshold(mRgba.getNativeObjAddr(), disp.getNativeObjAddr(), finalImage.getNativeObjAddr(), (int)converted_xcoord, (int)converted_ycoord);
			    	String colVal = String.valueOf(finalImage.cols());
			    	Log.d("Cols", colVal);
			    	Highgui.imwrite(leftimgFile.getAbsolutePath(), finalImage);
			    	myBitmap = BitmapFactory.decodeFile(leftimgFile.getAbsolutePath());
				    mImageView.setImageBitmap(myBitmap);
	 			}
	 			return false;
	 		}

	 	  }

    public native void getDisparity(long matAddrRgba, long matAddrfinalImage);
    public native void crop5(long matAddrRgba, long matAddrfinalImage);
    public native void getThreshold(long matAddrRgba, long matAddrDisp, long matAddrfinalImage, int ji1, int ji2);
}
