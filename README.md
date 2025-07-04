# Billboard - Qt6 Video Display System

Qt6 기반 비디오 디스플레이 시스템으로 디지털 사이니지나 안내 시스템용으로 설계되었습니다.

## 프로젝트 구조

```
Billboard/
├── src/           # C++ 소스 파일들
│   ├── main.cpp
│   ├── mainwindow.cpp
│   ├── videoplayer.cpp
│   ├── subtitle.cpp
│   └── infobox.cpp
├── include/       # 헤더 파일들
│   ├── mainwindow.h
│   ├── videoplayer.h
│   ├── subtitle.h
│   └── infobox.h
├── ui/            # Qt UI 파일들
│   └── mainwindow.ui
├── resources/     # 리소스 파일들
│   ├── data/      # 데이터 파일들
│   ├── videos/    # 비디오 파일들 (.mp4, .avi, .mkv 등)
│   └── subtitles/ # 자막 파일들
├── docs/          # 문서
├── build/         # 빌드 출력
├── CMakeLists.txt # CMake 빌드 파일
└── VideoPlayerApp.pro # Qt 프로젝트 파일
```

## 시스템 요구사항

- Qt6 (6.0 이상)
- GStreamer 1.0
- CMake 3.16 이상 또는 qmake
- C++11 지원 컴파일러

## 패키지 설치

### Ubuntu/Debian:
```bash
sudo apt update
sudo apt install -y \
    qt6-base-dev \
    qt6-multimedia-dev \
    libqt6multimedia6 \
    qmake6 \
    cmake \
    build-essential \
    libgstreamer1.0-dev \
    libgstreamer-plugins-base1.0-dev \
    pkg-config
```

### 라즈베리파이 추가 패키지:
```bash
sudo apt install -y \
    gstreamer1.0-plugins-good \
    gstreamer1.0-plugins-bad \
    gstreamer1.0-libav
```

## 빌드 방법

### CMake 사용 (권장):
```bash
cd Billboard
mkdir -p build && cd build
cmake ..
make -j$(nproc)
```

### qmake 사용:
```bash
cd Billboard
qmake6 VideoPlayerApp.pro
make -j$(nproc)
```

## 실행 방법

```bash
# CMake 빌드 후
./build/VideoPlayerApp

# qmake 빌드 후
./VideoPlayerApp
```

## 사용 방법

### 비디오 파일 추가
1. `resources/videos/` 폴더에 비디오 파일 복사
2. 지원 형식: MP4, AVI, MKV, MOV, WMV, FLV
3. 파일이 자동으로 감지되어 순환 재생됩니다

### 자막 설정
1. `resources/subtitles/subtitle.txt` 파일 생성
2. 한 줄에 하나씩 자막 텍스트 입력
3. 파일 변경 시 자동으로 업데이트됩니다

### 예배/서비스 스케줄
1. `resources/data/` 폴더에 스케줄 파일 생성
2. 파일 변경 시 실시간으로 정보창에 반영됩니다

## 주요 기능

- **3분할 레이아웃**: 비디오/자막/정보창
- **실시간 파일 감시**: 콘텐츠 자동 업데이트
- **전체화면 지원**: F11 토글, ESC 종료
- **라즈베리파이 최적화**: 하드웨어 가속 지원
- **반응형 UI**: 드래그로 영역 크기 조절

## 단축키

- `F11`: 전체화면 토글
- `ESC`: 전체화면 종료

## 구현 내용

### Qt5 → Qt6 마이그레이션

**주요 변경사항:**
- `QMediaPlaylist` 제거 → 직접 파일 목록 관리
- `QAudioOutput` 분리 → 오디오 출력 별도 관리
- `QRegExp` → `QRegularExpression` 교체
- `QTextCodec` 제거 → Qt6 기본 UTF-8 사용

**비디오 재생 구조:**
```cpp
// Qt6 방식
m_player = new QMediaPlayer(this);
m_audioOutput = new QAudioOutput(this);
m_player->setAudioOutput(m_audioOutput);

// 순환 재생 구현
connect(m_player, &QMediaPlayer::mediaStatusChanged,
        this, [this](QMediaPlayer::MediaStatus status) {
            if (status == QMediaPlayer::EndOfMedia) {
                playNext();
            }
        });
```

### 핵심 컴포넌트

**1. VideoPlayer**
- Qt6 QMediaPlayer 기반 비디오 재생
- 파일시스템 감시로 실시간 비디오 목록 업데이트
- 자동 순환 재생

**2. Subtitle**
- 텍스트 파일 기반 자막 시스템
- 5초 간격 자동 전환
- 실시간 파일 변경 감지

**3. InfoBox**
- 날짜/시간, 날씨, 서비스 정보 표시
- 1초 간격 시간 업데이트
- 분 단위 정보 갱신으로 성능 최적화

**4. MainWindow**
- QSplitter 기반 3분할 레이아웃
- 드래그 가능한 영역 크기 조절
- 전체화면 지원

### 파일 감시 시스템
```cpp
// QFileSystemWatcher로 실시간 업데이트
m_watcher = new QFileSystemWatcher(this);
connect(m_watcher, &QFileSystemWatcher::directoryChanged,
        this, &VideoPlayer::onDirectoryChanged);
```

### 라즈베리파이 최적화
```cpp
// main.cpp에서 하드웨어 가속 설정
#ifdef Q_OS_LINUX
    qputenv("QT_XCB_GL_INTEGRATION", "xcb_egl");
    qputenv("QT_GSTREAMER_USE_PLAYBIN_VOLUME", "true");
#endif
```

## 문제 해결

### 비디오가 재생되지 않는 경우:
```bash
# GStreamer 플러그인 확인
gst-inspect-1.0 | grep -i codec

# 추가 코덱 설치
sudo apt install gstreamer1.0-plugins-ugly
```

### 권한 문제:
```bash
# 실행 권한 부여
chmod +x VideoPlayerApp
```
