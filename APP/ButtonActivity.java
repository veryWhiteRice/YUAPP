package com.example.yuapp;

import androidx.appcompat.app.AppCompatActivity;
import java.io.*;
import java.net.*;
import android.os.Bundle;
import android.content.Intent;
import android.os.Handler;
import android.util.Log;
import android.view.View;
import android.widget.Button;
import android.widget.TextView;
import android.app.NotificationManager;
import java.io.IOException;
import android.app.PendingIntent;
import androidx.core.app.NotificationCompat;
import androidx.core.app.NotificationManagerCompat;
import android.app.NotificationChannel;
import android.content.Context;
import android.os.Build;
import androidx.core.content.ContextCompat;
public class ButtonActivity extends AppCompatActivity implements View.OnClickListener {
    final String TAG = "TAG+ButtonActivity";

    public InputStream dataInputStream;
    public OutputStream dataOutputStream;

    String IP = "172.20.10.9";    // 119 상황실 주소
    int port = 51000;
    SocketManager Clientsocket;
    private Handler mHandler;

    TextView show_text;             // 서버에서 온 거 보여주는 에디트
    TextView show_text2;
    SocketManager socketManager;
    Thread thread;
    int flag = -1;
    private class ReceiveThread extends Thread {
        private SocketManager socketManager;

        public ReceiveThread(SocketManager socketManager)
        {
            this.socketManager = socketManager;
        }

        @Override
        public void run() {
            try {
                while (true) {
                    String receivedMessage = socketManager.receiveMessage(); // 소켓에서 메시지 수신
                    mHandler.post(new DisplayMessageRunnable(receivedMessage)); // 메인 쓰레드에게 메시지 전달
                    showNotification(receivedMessage); // 알림 메시지 표시
                }
            } catch (IOException e) {
                Log.e(TAG, "Error receiving message: " + e.getMessage());
            }
        }
    }

    private class ReceiveThread2 extends Thread
    {
        private SocketManager Clientsocket;

        public ReceiveThread2(SocketManager Clientsocket)
        {
            this.Clientsocket = Clientsocket;
        }
        public void run() {
            try {
                while (true) {
                    String receivedMessage = Clientsocket.receiveMessage(); // 소켓에서 메시지 수신
                    mHandler.post(new DisplayMessageRunnable2(receivedMessage)); // 메인 쓰레드에게 메시지 전달
                }
            } catch (IOException e) {
                Log.e(TAG, "Error receiving message: " + e.getMessage());
            }
        }

    }

    private class DisplayMessageRunnable implements Runnable {
        private String message;

        public DisplayMessageRunnable(String message)
        {
            this.message = message;
        }

        @Override
        public void run()
        {
            Log.i(TAG, "Displaying message: " + message);
            show_text.setText(message); // 텍스트 뷰에 메시지 표시
            new Thread(new Runnable() {
                @Override
                public void run() {
                    Clientsocket.sendMessage("영남");
                }
            }).start();
        }
    }

    private class DisplayMessageRunnable2 implements Runnable {
        private String message;

        public DisplayMessageRunnable2(String message)
        {
            this.message = message;
        }

        @Override
        public void run()
        {
            Log.i(TAG, "Displaying message: " + message);
            show_text2.setText(message); // 텍스트 뷰에 메시지 표시
        }
    }
    private void initializeSocket() {
        String roomIP = getIntent().getStringExtra("ip");
        int roomPort = getIntent().getIntExtra("port", -1);
        new Thread(new Runnable() {
            @Override
            public void run() {
                socketManager = new SocketManager(roomIP, roomPort); // 라즈베리파이와 통신하는 소켓
                thread = new ReceiveThread(socketManager);
                thread.start();
            }
        }).start();

        new Thread(new Runnable() {
            @Override
            public void run() {
                Clientsocket = new SocketManager("172.20.10.2", 51000); // 119 서버와 통신하는 소켓
                thread = new ReceiveThread2(Clientsocket);
                thread.start();
            }
        }).start();
    }
    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.main2);

        mHandler = new Handler();
        initializeSocket();

        Button button = findViewById(R.id.button);
        button.setOnClickListener(this);

        Button button2 = findViewById(R.id.button2);
        button2.setOnClickListener(this);

        Button button3 = findViewById(R.id.button3);
        button3.setOnClickListener(this);

        Button button4 = findViewById(R.id.button4);
        button4.setOnClickListener(this);

        show_text = findViewById(R.id.text_view);
        show_text2 = findViewById(R.id.text_view2);
        TextView panTextView = findViewById(R.id.pan_text_view);
        TextView sensorTextView = findViewById(R.id.sensor_text_view);
        panTextView.setText("PAN: 상태를 확인하세요");
        sensorTextView.setText("SENSOR: 상태를 확인하세요");
    }

    @Override
    public void onClick(View v) {
        TextView panTextView = findViewById(R.id.pan_text_view);
        TextView sensorTextView = findViewById(R.id.sensor_text_view);
        if (v.getId() == R.id.button) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    socketManager.sendMessage("PAN_ON");
                }
            }).start();
            panTextView.setText("PAN: ON");
            int colorOn = ContextCompat.getColor(this, R.color.lightBlue);
            panTextView.setTextColor(colorOn);
        } else if (v.getId() == R.id.button2) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    socketManager.sendMessage("PAN_OFF");
                }
            }).start();
            panTextView.setText("PAN: OFF");
            int colorOff = ContextCompat.getColor(this, R.color.lightPink);
            panTextView.setTextColor(colorOff);
        } else if (v.getId() == R.id.button3) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    socketManager.sendMessage("SENSOR_ON");
                }
            }).start();
            sensorTextView.setText("SENSOR: ON");
            int colorOn = ContextCompat.getColor(this, R.color.lightBlue);
            sensorTextView.setTextColor(colorOn);
        } else if (v.getId() == R.id.button4) {
            new Thread(new Runnable() {
                @Override
                public void run() {
                    socketManager.sendMessage("SENSOR_OFF");
                }
            }).start();
            sensorTextView.setText("SENSOR: OFF");
            int colorOff = ContextCompat.getColor(this, R.color.lightPink);
            sensorTextView.setTextColor(colorOff);
        }
    }

    // 알림 매니저
    NotificationManager notificationManager;

    // 알림 채널 ID
    String channelId = "my_channel_id";

    // 알림 채널 이름
    CharSequence channelName = "My Channel";

    // 알림 채널 중요도
    int importance = NotificationManager.IMPORTANCE_HIGH;

    // 알림 메시지를 표시하는 메서드
    private void showNotification(String message) {
        // 알림을 클릭했을 때 실행되는 인텐트 설정
        Intent intent = new Intent(this, MainActivity.class);
        PendingIntent pendingIntent = PendingIntent.getActivity(this, 0, intent, PendingIntent.FLAG_UPDATE_CURRENT);

        // 알림 빌더 생성 및 설정
        NotificationCompat.Builder builder = new NotificationCompat.Builder(this, channelId)
                .setSmallIcon(R.drawable.notification_icon)
                .setContentTitle("긴 급")
                .setContentText(message)
                .setPriority(NotificationCompat.PRIORITY_HIGH)
                .setContentIntent(pendingIntent)
                .setAutoCancel(true);

        // 알림 채널 생성
        if (Build.VERSION.SDK_INT >= Build.VERSION_CODES.O) {
            NotificationChannel channel = new NotificationChannel(channelId, channelName, importance);
            notificationManager = getSystemService(NotificationManager.class);
            notificationManager.createNotificationChannel(channel);
        }

        // 알림 생성
        notificationManager.notify(0, builder.build());
    }
}
