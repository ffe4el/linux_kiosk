# linux_kiosk
🍿linux_final_project_Theater

### 영화관 키오스크 개발
- 서버 클라이언트 (s erver client) 모델로 동작하는 키오스크 시스템
- 서버는 키오스크를 관리하는 역할을 수행하며 클라이언트는 키오스크 의 고객 역
할을 수행
- 서버는 키오스크 관리 상품과 각 상품의 수량을 관리 클라이언트들에게 서비스
제공 을 할 수 있도록 함
- 클라이언트는 키오스크 사용을 수행

### 기본 기능
- 서버
  ◼ 사용자에게 키오스크 에 대한 정보를 제공 상 품 의 종 류 및 각 상 품 의 가 격 ,
  수 량 등
  ◼ 각 상품에 대한 수량 관리
  ◼ 수량 초과 시 클라이언트가 상품을 가져갈 수 없도록 함
  ◼ 수량이 충분하면 클라이언트가 상품 수령할 수 있도록 함
  ◼ 상 품 총 금 액 과 지 불 금 액 을 비 교 하여 각 사 용 자에 대 한 계 산 수 행
- 클라이언트
  ◼ 원하는 상품 에 접근
  ◼ 상품의 개수 입력
  ◼ 필 요 한 상 품 을 다 고 른 뒤에 금 액 지 불

### 프로젝트 내 추가 구현 기능
-	여러 사용자가 동시에 서버에 접속 가능합니다. (최대 10개의 클라이언트가 동시 접속 가능하도록 설정)
-	한 클라이언트에서 구매하면 다른 클라이언트에서 상품에 대한 정보가 실시간으로 반영됩니다. (fwrite, fread, fseek 이용)
-	서버를 열 때, 관리자모드에서 각 상품 및 상품의 개수, 가격, 수량 설정 가능합니다. (동적할당 malloc을 이용)
-	각 클라이언트의 키오스크 사용시간에 제한을 120초로 설정했습니다. (alarm을 이용)
-	관리자 모드일때 동안은 client에서의 연결 접속을 거부하고 키오스크를 이용할 수 없도록 했습니다.

### 본 프로젝트의 강점
-	영화관과 매점(음식점)을 모두 구현한 부분에서 추후 다른 분야도 추가 결합하기 용이한 코드임을 확인했습니다. (little bit…함수의 모듈화)
-	디테일한 예외처리로 인해 부드러운 사용이 가능합니다. 
-	직관적인 UI (영화 리스트, 음식 리스트, 영화 좌석표 등등) 을 통하여 처음 사용하는 사람도  이용하기 쉽게 구성하였습니다.
-	코드마다 주석 처리를 꼼꼼히 하여 16팀의 코드를 체크하시는 교수님의 안구의 손상을 덜 수 있도록 노력했습니다.


### 프로젝트 흐름도
  <img width="445" alt="image" src="https://github.com/ffe4el/linux_kiosk/assets/93892724/78957cf7-1c66-4170-9075-4033cf8084f7">


### 프로젝트 콘솔 명령어 README
프로젝트 콘솔 명령어 README 파일 경로 : ./final/README.txt 




