# ps2avrU

## ps2avrGB란?

1. USB / PS/2 콤보 펌웨어입니다.
2. PS/2 키보드 펌웨어인 ps2avr과 USB키보드 펌웨어인 dulcimer를 조합하여 만든 펌웨어입니다.

프로젝트 링크
- ps2avr : <http://ps2avr.sourceforge.net>
- dulcimer : <http://www.schatenseite.de/dulcimer.html?L=2>

> ps2avr 시리즈의 기판은 확장 모듈(ps2avr-ext)를 함께 사용해야 USB인터페이스를 이용할 수 있습니다.


**ps2avrGB**는 ps2avrU 펌웨어에 RGB LED를 컨트롤할 수 있는 Sub MCU를 추가한 펌웨어 입니다. 
대부분의 기능은 ps2avrU와 동일하며 추가적으로 더 손쉽게 키 맵핑 및 커스텀 매크로 등을 등록할 수 있는 Bootmapper Client를 이용한 관리가 가능합니다.

> 현재 ps2avrU는 더이상 지원이 중단 된 상태이며, 이를 대신하는 "ps2avrGB4U"가 배포되고 있습니다.

## 인터페이스의 차이점

USB
- 6+1키 동시입력
- 보통의 키 반복 속도

PS/2 
- 무한 동시입력
- 빠른 키 반복 속도


## 인터페이스 전환 방법

기본값은 USB로 설정되어있습니다.
* V1.2.4 이전
    - PS/2 로 선택하려면 "P"키를 누른 상태에서 PS/2 포트에 연결합니다.
(보통 PS/2 포트는 핫플러그를 지원하지 않기 때문에 USB 포트에 PS/2모드로 선택만 한 후, 컴퓨터 전원 off -> PS/2 연결 -> 부팅 순으로 이용하시는 것이 좋습니다.)
    - 이렇게 수동으로 인터페이스를 선택하면 이후에 재 접속해도 다시 그 인터페이스로만 작동을 합니다.
    - USB로 설정하려면 "U"키를 누른 상태에서 USB포트에 연결하면 됩니다.
* V1.3.0 이후
    - 키 입력을 통한 전환 방법은 제거
    - Bootmapper Client를 이용해서 전환하는 기능 추가    
    - 부트로더로 연결 후 “Set USB”또는 “Set PS/2″를 클릭해서 전환 

인터페이스 인식 표시는 CapsLock/NumLock LED의 깜박이는 수로 확인 가능합니다.
- 1번 깜박인 후 키보드 동작 : PS/2로 인식된 상태
- 깜박임 없이 키보드 동작 : USB로 인식된 상태


## RGB LED 지원

컬러를 자유롭게 변경할 수 있는 RGB LED를 지원합니다.
이에 관한 모든 설정은 "Bootmapper Client"에서 할 수 있고, 밝기와 모드는 키보드에서도 직접 변경이 가능합니다.

- 모드 변경 
    - ESC + SHIFT + Caps lock 
    - RGB MODE 키 (또는 SHIFT + LED MODE 키)

> PS/2 연결시 RGB LED가 off 일 경우에만 스위치쪽 full LED가 점등 됩니다.

- RGB key event mode 변경
    - ESC + Ctrl + Capslock으로 RGB key event mode 변경
    - Ctrl + RGB MODE 키


- 밝기 변경
    - SHIFT + LED BR UP 키
    - SHIFT + LED BR DOWN 키

> RGB LED의 밝기가 밝아질 수록 full LED는 어두워집니다.

> full LED를 최대 밝기로 이용하고자 할 경우 RGB LED의 밝기는 중간 이하로 설정해야 합니다.

> 컬러 및 기타 세부 설정은 "Bootmapper Client"를 통해서 할 수 있습니다.
> Bootmapper Client 사용 법 : <http://blog.winkeyless.kr/154>



## 스위치 full LED

LED 모드 변경 단축키
- **ESC + caps lock**

1. LED off -  꺼짐
2. fading - 숨쉬기
3. on - 계속 켜짐
4. key down level up - 키 입력이 잦을수록 밝아짐
5. key down level down - 키 입력이 잦을수록 어두워짐

Full LED의 최대 밝기를 조절할 수 있는 기능 키
- LED BR UP : 최대 밝기 증가
- LED BR DOWN : 최대 밝기 감소

스위치 full LED의 조절 키들은 기본적으로 다음의 위치에 매핑되어 있습니다.

- LED MODE : LED 모드를 순차적으로 변경합니다.
    - FN+Caps lock
    
- 밝기를 조절합니다.
    + B.87, B.mini 등 F1~12키가 있는 기판 
        + LED BR UP -> FN + =
        + LED BR DOWN -> FN + -

    + B.face B.thumb 등 F1~12키가 없는 기판 
        + LED BR UP -> FN + H
        + LED BR DOWN -> FN + G


> 스위치 LED는 RGB LED의 밝기에 따라서 최대 밝기가 제한 됩니다.


## 4 Layer 키맵
1. normal
2. FN
    - FN키를 누르고 있는 동안 작동되는 레이어
3. FN2/FN3
    - FN2/FN3키를 누르고 있는 동안 FN2/FN3레이어가 유지됩니다.
    - "TOGGLE_FN2/TOGGLE_FN3"키로 매핑된 키를 이용하면 FN2/FN3레이어로 유지됩니다. 
    - toggle on/off를 caps lock/num lock LED의 깜박임 수로 표시합니다. on/off 각 2번/1번 점멸 합니다.
    - Bootmapper Client의 옵션을 설정하여 N/C/S LED를 FN2/FN3의 인디케이터로 대신 사용할 수 있습니다.
    - normal 레이어로 복귀하려면 다시 "TOGGLE_FN2/TOGGLE_FN3" 키를 다시 누르거나 **ESC+Backspace**를 누르면 됩니다.


## 전용 소프트웨어 Bootmapper Client 지원
Bootmapper Client(부트맵퍼 클라이언트)를 이용해서 키맵핑/매크로 설정/듀얼액션 설정/옵션 설정 등을 할 수 있습니다.
> Bootmapper Client 사용 법 : http://blog.winkeyless.kr/154

## 퀵 스왑 기능
- 퀵 스왑 적용(ALT <-> GUI) : **ALT+GUI+R shift** 를 동시에 누르고 5초간 유지하면 두 키(ALT <-> GUI)의 위치가 뒤바뀝니다. (적용시 캡스락/넘락 LED가 2회/1회 깜빡이며 표시)


## PS/2 키 반복 속도 조절
- Bootmapper Client 의 Options 탭에서 조절 가능합니다.

## Dual action key 
이 키들은 키 기본적으로 조합 키로 작동합니다. 다만, 다른키와 조합하지 않고 하나만 눌렀다 때면(down->up) 한/영 이나 한자 등의 키가 입력됩니다.
> 예를 들어, "FN + Capslock" 키를 누른 상태에서 다른 키들을 누르면 FN 레이어 키로 작동하지만, 이 키만 단독으로 눌렀다 때면 Capslock 키로 작동합니다. 

- 부트맵퍼 클라이언트를 통해서 여러가지 조합의 듀얼 액션 키를 설정 할 수 있습니다.

- dual action key 작동 방식
    - 누른 상태로 잠시(약 0.5초) 있어야 조합키가 작동된다.
    - 다른 키와 조합하면 딜레이와 관계없이 조합키로 작동된다.
    - 단독 키(한/영 등)로 사용하려면 눌렀다 떼는 행동을 약0.5초 안에 해야 된다.

    > 예를 들어 "(R alt + 한/영)"키를 이용시 한/영 을 사용하고 싶다면 0.5초 내에 다운/업을 해야 하고, R alt를 누르고 싶다면 0.5초 동안 누르고 있거나, 다른 키들과 조합을 하면 된다.

> 윈도우즈의 키보드 설정이 101키로 되어 있는 경우 R Alt가 한/영, RCtrl이 한자 키로 작동하기 때문에 위 키들 중에 해당 사항이 있는 키(R alt + 한/영 등)는 듀얼 액션으로 작동되지 않습니다.


## Lock Key/Win 키
- Lock Key 키는 키보드의 모든 입력을 차단합니다.
- Lock Win 키는 좌/우 윈도우 키의 입력을 차단합니다.
- 두 키 모두 토글 방식으로 작동하며, 전원이 차단되면 모두 off 상태(차단하지 않는 상태)가 됩니다.


##  Shift + Esc = ~
- 미니키보드에서 입력이 불편했던 ~를 Shift + Esc를 누르면 입력되도록 키 매퍼 옵션 추가
- 하드웨어 메뉴의 "esc to ~" 옵션을 "on"으로 변경하면 shift+esc 입력시 ~가 입력됩니다.

## 디바운스 타이밍 조절 가능
- 최적의 키입력을 위한 디바운스 타이밍 조절 기능 추가

> 디바운스 타이밍을 빠르게 하면 스위치의 상태에 따라서 중복 입력 현상이 나타나기도 합니다. 그런 경우 다시 디바운스 값을 높혀주면 됩니다.

## 펌웨어 다운로드 링크

<https://github.com/showjean/ps2avrU/releases>

## 부트로더 링크
https://blog.winkeyless.kr/152

## Store Site
Global : <http://winkeyless.kr>
한국 : <http://winkeyless.com>
