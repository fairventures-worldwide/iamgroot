package com.lae.iamgroot;

import android.content.Intent;
import android.content.pm.PackageManager;
import android.content.res.AssetManager;
import android.graphics.Bitmap;
import android.graphics.BitmapFactory;
import android.net.Uri;
import android.os.Bundle;
import android.os.Environment;
import android.provider.MediaStore;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.widget.Toast;

import androidx.annotation.NonNull;
import androidx.appcompat.app.AppCompatActivity;
import androidx.core.app.ActivityCompat;
import androidx.core.content.ContextCompat;
import androidx.core.content.FileProvider;

import com.lae.iamgroot.utils.ImageProcessing;

import org.opencv.android.OpenCVLoader;
import org.opencv.android.Utils;
import org.opencv.core.Mat;

import java.io.File;
import java.io.FileNotFoundException;
import java.io.IOException;
import java.io.InputStream;
import java.text.SimpleDateFormat;
import java.util.Date;

public class MainActivity extends AppCompatActivity {
    private static String TAG = "MainActivity";
    // Used to load the 'native-lib' library on application startup.
    String currentPhotoPath;
    private static final int MY_CAMERA_PERMISSION_CODE = 100;
    static final int REQUEST_IMAGE_CAPTURE = 1888;

    private final int CAMERA_REQUEST_CODE = 2;
    private final int STORAGE_REQUEST_CODE = 5;
    ImageProcessing imageProcessing;
    TextView tv;

    static {
        System.loadLibrary("native-lib");
        System.loadLibrary("opencv_java4");
    }

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.activity_main);
        imageProcessing = new ImageProcessing();
        Button bt = findViewById(R.id.sample_button);
        tv = findViewById(R.id.textView);

        bt.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View v) {

                Intent a = new Intent(MainActivity.this, CameraActivity.class);
                startActivityForResult(a, REQUEST_IMAGE_CAPTURE);
            }
        });


        if (OpenCVLoader.initDebug()) {
            Log.d(TAG, "OPENCV SUCCESSFULY LOAD");
        } else {
            Log.d(TAG, "OPENCV DİD NOT LOAD");

        }
    }


    private void dispatchTakePictureIntent() {
        Intent takePictureIntent = new Intent(MediaStore.ACTION_IMAGE_CAPTURE);
        // Ensure that there's a camera activity to handle the intent
        if (takePictureIntent.resolveActivity(getPackageManager()) != null) {
            // Create the File where the photo should go
            File photoFile = null;
            try {
                photoFile = createImageFile();
            } catch (IOException ex) {
                // Error occurred while creating the File
            }
            // Continue only if the File was successfully created
            if (photoFile != null) {
                Uri photoURI = FileProvider.getUriForFile(this,
                        "com.lae.iamgroot.fileprovider",
                        photoFile);
                takePictureIntent.putExtra(MediaStore.EXTRA_OUTPUT, photoURI);
                startActivityForResult(takePictureIntent, REQUEST_IMAGE_CAPTURE);
            }
        }
    }


    private void askPermission(String permission, int requestCode) {
        if (ContextCompat.checkSelfPermission(this, permission) != PackageManager.PERMISSION_GRANTED) {
            // We Dont have permission
            ActivityCompat.requestPermissions(this, new String[]{permission}, requestCode);

        } else {
            // We already have permission do what you want
            Toast.makeText(MainActivity.this, "Permission is already granted.", Toast.LENGTH_LONG).show();
        }

    }

    @Override
    public void onRequestPermissionsResult(int requestCode, @NonNull String[] permissions, @NonNull int[] grantResults) {
        switch (requestCode) {
            case CAMERA_REQUEST_CODE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(this, "Camera Permission Granted", Toast.LENGTH_LONG).show();

                } else {
                    Toast.makeText(this, "Camera Permission Denied", Toast.LENGTH_LONG).show();
                }
                break;
            case STORAGE_REQUEST_CODE:
                if (grantResults.length > 0 && grantResults[0] == PackageManager.PERMISSION_GRANTED) {
                    Toast.makeText(this, "Storage Permission Granted", Toast.LENGTH_LONG).show();

                } else {
                    Toast.makeText(this, "Storage Permission Denied", Toast.LENGTH_LONG).show();
                }
                break;
        }

    }

    private File createImageFile() throws IOException {
        // Create an image file name
        String timeStamp = new SimpleDateFormat("yyyyMMdd_HHmmss").format(new Date());
        String imageFileName = "JPEG_" + timeStamp + "_";
        File storageDir = getExternalFilesDir(Environment.DIRECTORY_PICTURES);
        File image = File.createTempFile(
                imageFileName,  /* prefix */
                ".jpg",         /* suffix */
                storageDir      /* directory */
        );

        // Save a file: path for use with ACTION_VIEW intents
        currentPhotoPath = image.getAbsolutePath();
        return image;
    }


    public void onActivityResult(int requestCode, int resultCode, Intent data) {
        super.onActivityResult(requestCode, resultCode, data);
        if (requestCode == REQUEST_IMAGE_CAPTURE && resultCode == RESULT_OK) {
            //Check Android version, as intent.getData() will return data only if v is above or equal to M otherwise the data will be null

            try {
                final Uri imageUri = Uri.parse(data.getStringExtra("imgUri"));
                final InputStream imageStream = getContentResolver().openInputStream(imageUri);
                final Bitmap selectedImage = BitmapFactory.decodeStream(imageStream);
                Bitmap bitmapImg = selectedImage.copy(Bitmap.Config.ARGB_8888, true);
                Mat src = new Mat();

                Utils.bitmapToMat(bitmapImg, src);

//                Bitmap cardFile = imageProcessing.getCardFile(this.getApplicationContext());
//                Bitmap bitmapCard = cardFile.copy(Bitmap.Config.ARGB_8888,true);
//                Mat matCard = new Mat();
//                Utils.bitmapToMat(bitmapCard,matCard);

                long start = System.nanoTime();
                double diameter = measureTree(src.getNativeObjAddr());
                long end = System.nanoTime();
                int intValue = (int) Math.round(diameter);

                tv.setText("Diameter: " + Math.round(intValue) + " - Elapsed Time in ms: " + (end - start) / 1000000);


                Log.d(TAG, "DIAMETER " + diameter + "/n Elapsed Time in Nano sec : " + (end - start) + "/n Elapsed Time in millisec: " + (end - start) / 1000000);

            } catch (FileNotFoundException e) {
                e.printStackTrace();

                tv.setText("Cannot Measure Tree");
            }

        }
    }


    public AssetManager getAssetManager() {
        return getAssets();
    }

    public native double measureTree(long mat);

}