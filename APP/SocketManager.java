package com.example.yuapp;
import java.io.*;
import java.net.*;
import android.util.Log;
public class SocketManager {
    private static SocketManager instance;
    final String TAG = "TAG+SocketManager";
    private Socket socket;
    private final String serverIP = "172.20.10.3";//라즈베리파이 주소
    private final int serverPort = 50000;

    /*private SocketManager()
    {
        // 소켓 초기화
        try {
            socket = new Socket(serverIP, serverPort);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }*/
    public  SocketManager(String IP, int port)
    {
        // 소켓 초기화
        try {
            socket = new Socket(IP, port);
        } catch (IOException e) {
            e.printStackTrace();
        }
    }

    /*public static synchronized SocketManager getInstance() {
        if (instance == null) {
            instance = new SocketManager();
        }
        return instance;
    }
    public static synchronized SocketManager getInstance(String IP, int port) {
        if (instance == null) {
            instance = new SocketManager(IP, port);
        }
        return instance;
    }*/
    public void sendMessage(String message) {
        // 소켓을 이용하여 메세지 전송
        try {
            OutputStream outputStream = socket.getOutputStream();
            outputStream.write((message).getBytes("EUC-KR"));
            outputStream.flush();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
    public String receiveMessage() throws IOException {
        try {
            InputStream inputStream = socket.getInputStream();
            BufferedReader reader = new BufferedReader(new InputStreamReader(inputStream));
            String message = reader.readLine();
            Log.i(TAG, "Received message: " + message);
            return message;
        } catch (IOException e) {
            e.printStackTrace();
        }
        return null;
    }

    public void closeSocket() {
        // 소켓 연결 종료
        try {
            socket.close();
        } catch (IOException e) {
            e.printStackTrace();
        }
    }
}
