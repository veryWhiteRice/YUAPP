package com.example.yuapp;
import androidx.appcompat.app.AppCompatActivity;

import android.os.Bundle;
import android.content.Intent;

import android.view.View;
import android.widget.Button;

import android.widget.*;
import java.util.*;

public class roomActivity extends AppCompatActivity
{
    private LinearLayout container;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.room);

        container = findViewById(R.id.container);

        // 호 버튼들을 동적으로 생성
        createRoomButtons();
    }

    private void createRoomButtons() {
        // 호 목록을 가져오는 로직을 구현해야 함
        List<String> roomList = getRoomList(); // 호 목록을 가져오는 메소드 호출

        // 각 호 버튼을 생성하고 클릭 이벤트 처리
        for (String room : roomList) {
            LinearLayout roomLayout = new LinearLayout(this);
            roomLayout.setOrientation(LinearLayout.VERTICAL);

            // Create an ImageView for the room image
            ImageView imageView = new ImageView(this);
            imageView.setPadding(0, 0, 0, 30); // 패딩 제거

            // Set the image source here. You might need to adjust this depending on where your images are.
            imageView.setImageResource(R.drawable.room_image); // 'room_image' is the placeholder for your image resource.
            imageView.setScaleType(ImageView.ScaleType.FIT_XY); // 스케일 타입 조정
            LinearLayout.LayoutParams imgParams = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.WRAP_CONTENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
            );
            imgParams.setMargins(0, 0, 0, 0); // 마진 제거

            imageView.setLayoutParams(imgParams);
            roomLayout.addView(imageView);
            Button button = new Button(this);
            button.setText(room);
            button.setPadding(0,0,0,0);
            LinearLayout.LayoutParams btnParams = new LinearLayout.LayoutParams(
                    LinearLayout.LayoutParams.MATCH_PARENT,
                    LinearLayout.LayoutParams.WRAP_CONTENT
            );
            btnParams.bottomMargin = 100;  // adjust the value here
            button.setLayoutParams(btnParams);
            button.setOnClickListener(new View.OnClickListener() {
                @Override
                public void onClick(View v) {
                    // 호 버튼이 클릭되었을 때 실행되는 로직
                    handleRoomButtonClick(room);
                }
            });
            roomLayout.addView(button);
            container.addView(roomLayout);
        }
    }

    private void handleRoomButtonClick(String room) {
        // 예시로 호를 클릭했을 때 해당 호 번호를 다음 activity로 전달하여 연결된 소켓과 함께 사용할 수 있습니다.
        Intent intent = new Intent(roomActivity.this, ButtonActivity.class);
        intent.putExtra("room", room);
        intent.putExtra("ip", "172.20.10.9"); // 선택된 방의 IP
        intent.putExtra("port", 50000); // 선택된 방의 포트
        startActivity(intent);
    }

    // 호 목록을 가져오는 메소드 (임시로 예시로 만듦)
    private List<String> getRoomList() {
        List<String> roomList = new ArrayList<>();
        roomList.add("101호");
        roomList.add("102호");
        roomList.add("103호");
        roomList.add("104호");
        roomList.add("105호");
        // ...
        return roomList;
    }
}

