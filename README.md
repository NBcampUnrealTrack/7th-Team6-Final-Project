<div align="center">

# Part-Time Beat!!

**비트타운에서 아르바이트하며 여행 자금을 모으는 3D 캐주얼 리듬 게임**

![Unreal Engine](https://img.shields.io/badge/Unreal_Engine-5.6-0E1128?logo=unrealengine&logoColor=white)
![C++](https://img.shields.io/badge/C++-00599C?logo=cplusplus&logoColor=white)
![Blueprint](https://img.shields.io/badge/Blueprint-137CBD?logo=unrealengine&logoColor=white)
![Wwise](https://img.shields.io/badge/Audio-Wwise-00549F)
![Platform](https://img.shields.io/badge/Platform-PC-0078D6?logo=windows&logoColor=white)
![Players](https://img.shields.io/badge/Players-Single_Player-orange)

[게임 영상](https://youtu.be/5rShTf2_pOs?si=eAFhhQbZcBBJ-dPV) · [실행 빌드](https://drive.google.com/file/d/1pALNzBV3WtzKl0bubRrCOdZph3jbSF6U/view?usp=sharing) · [프로젝트 브로셔](https://www.notion.so/teamsparta/7-6-Part-Time-Beat-3792dc3ef514808d9bced8a2b6916207)

<a href="https://youtu.be/5rShTf2_pOs?si=eAFhhQbZcBBJ-dPV">
  <img src="https://img.youtube.com/vi/5rShTf2_pOs/maxresdefault.jpg" alt="Part-Time Beat 게임 영상" width="720" />
</a>

</div>

## 프로젝트 소개

Part-Time Beat!!는 하나의 월드에서 여러 종류의 리듬 미니게임을 선택해 플레이하는 3D 캐주얼 리듬 게임입니다.

플레이어는 음악과 리듬이 일상의 중심인 **비트타운**에 사는 평범한 고등학생입니다. 다가오는 여름방학에 열대 섬으로 여행을 떠나기 위해, 비트에 맞춰 다양한 아르바이트에 도전하고 여행 자금을 모읍니다.

귀여운 캐주얼 비주얼과 미니게임 모음집의 접근성을 결합하고, 리듬 게임을 처음 접하는 플레이어부터 높은 난도를 원하는 플레이어까지 함께 즐길 수 있는 게임을 목표로 제작했습니다.

| 항목 | 내용 |
| --- | --- |
| 장르 | 3D 캐주얼 리듬 게임 |
| 플랫폼 | PC |
| 플레이 인원 | 1인 |
| 엔진 | Unreal Engine 5.6 |
| 개발 언어 | C++, Blueprint |
| 오디오 | Audiokinetic Wwise |
| 기본 브랜치 | `develop` |

## 주요 특징

### 여러 리듬 미니게임을 하나의 월드에서

월드맵을 돌아다니며 미니게임 입구에 접근하면 게임 설명을 확인하고 원하는 아르바이트를 선택할 수 있습니다. 프로젝트에는 `BB`, `CH`, `DW`, `FOD`, `FS`, `JJ`, `LC`, `PC`, `SR`로 구분된 9개의 미니게임 콘텐츠가 구성되어 있으며, 각 게임은 독립적인 레벨·채보·데이터 에셋·UI·오디오 이벤트를 가집니다.

### 세 단계 난이도

| 난이도 | 설명 |
| --- | --- |
| Easy | 리듬 게임 입문자를 위한 쉬운 모드 |
| Standard | 기본 플레이 경험을 제공하는 일반 모드 |
| Insane | 숙련자를 위한 고난도 모드 |

### 공통 리듬 판정 파이프라인

미니게임마다 로직을 중복 구현하지 않고, 공통 리듬 시스템을 기반으로 채보 재생부터 점수 계산까지 처리합니다.

```text
.rhythmc 채보(JSON)
        ↓
Chart Parser / Chart Asset
        ↓
Rhythm Conductor ── Wwise 재생 위치 동기화
        ↓
Cue → Arm → Note Reached
        ↓
Judgement System
        ↓
Score / Combo / Grade
```

- 전용 채보 제작기로 제작한 `.rhythmc` JSON 채보 파싱
- Wwise의 실제 재생 시간을 기준으로 노트·비트·마디 이벤트 발행
- `HighPerfect`부터 `Miss`까지 입력 타이밍 판정
- 점수, 콤보, 등급, 별점 계산
- 입력·화면·사운드 오프셋의 분리 보정
- 메트로놈 입력을 이용한 지연 시간 자동 캘리브레이션

### 플레이어 편의 기능

- 프로필 생성·선택·삭제 및 자동 저장
- 그래픽·사운드·게임 옵션 설정
- 단축키 변경
- 판정 오프셋 설정 및 싱크 조절
- 미니게임 재도전과 월드맵 복귀
- 난이도별 기록과 결과 화면

## 기술 스택

| 분류 | 기술 |
| --- | --- |
| Game Engine | Unreal Engine 5.6 |
| Programming | C++ / Blueprint |
| IDE | Visual Studio 2022 / JetBrains Rider |
| Audiokinetic Wwise |
| UI | UMG / Slate |
| Input | Enhanced Input |
| VFX | Niagara |
| Data | JSON / DataAsset / DataTable |
| Version Control | Git / GitHub / Git LFS |

## 핵심 시스템

### Rhythm

- `PTBRhythmChartAsset`: 파싱된 노트와 BPM, 오프셋, Wwise 이벤트 메타데이터 보관
- `PTBRhythmChartParser`: `.rhythmc` 파일과 JSON 문자열을 런타임 채보 데이터로 변환
- `PTBRhythmConductorComponent`: Wwise 재생 시간을 기준으로 노트 Cue, Arm, 도달, Beat, Bar 이벤트 발행
- `PTBJudgementSystem`: 채보 노트와 입력 시간을 비교해 판정 생성
- `PTBScoreCalculator`: 판정 결과를 누적해 점수, 콤보, 등급과 별점 계산

### Audio

- `PTBWwiseAudioManager`: BGM/SFX 재생, RTPC·State·Switch 제어, SoundBank 로드
- `PTBWwiseEventMapAsset`: BGM, 판정음, UI, 미니게임 이벤트 매핑
- `PTBWwiseRhythmSyncComponent`: Wwise 재생 위치를 채보 시간과 Beat로 환산하고 지연 보정

### Mini Game Framework

- `PTBBaseMiniGame`: 공통 라운드 생명주기와 재도전 시 액터·위젯 정리
- `PTBMiniGameRuleSet`: 판정 범위, 실패 조건, 인트로·아웃트로 시간과 오입력 처리 규칙
- 공통 시스템과 게임별 연출을 분리해 새로운 미니게임을 확장할 수 있도록 구성

### Profile & Settings

- 프로필 CRUD와 활성 프로필 관리
- 닉네임 유효성 및 금칙어 검사
- 그래픽, 사운드, 키 바인딩, 리듬 오프셋 저장
- 게임 흐름에 따른 자동 저장

## 프로젝트 구조

```text
PartTimeBeat/
├─ Config/
├─ Content/
│  ├─ PTB/
│  │  ├─ Core/
│  │  ├─ Characters/
│  │  ├─ World/
│  │  ├─ Maps/
│  │  ├─ Data/
│  │  ├─ MiniGames/
│  │  │  ├─ Common/
│  │  │  ├─ BB/  CH/  DW/  FOD/  FS/
│  │  │  └─ JJ/  LC/  PC/  SR/
│  │  ├─ UI/
│  │  └─ Movies/
│  ├─ PaidAssets/
│  ├─ FreeAssets/
│  └─ WwiseAudio/
├─ Plugins/
│  └─ Wwise/
├─ Source/
│  └─ PartTimeBeat/
│     ├─ Core/
│     ├─ Save/
│     ├─ Settings/
│     ├─ Profile/
│     ├─ Rhythm/
│     ├─ Audio/
│     ├─ MiniGames/
│     ├─ UI/
│     ├─ Characters/
│     ├─ Tutorial/
│     └─ Progression/
├─ WwiseProject/
└─ PartTimeBeat.uproject
```

## 시작하기

### 실행 빌드로 플레이

소스 빌드 없이 게임만 플레이하려면 아래 배포 파일을 내려받아 실행합니다.

- [PartTimeBeat-FinalBuild5.zip](https://drive.google.com/file/d/1pALNzBV3WtzKl0bubRrCOdZph3jbSF6U/view?usp=sharing)

### 개발 환경에서 실행

#### 요구 사항

- Unreal Engine **5.6**
- Visual Studio 2022 또는 JetBrains Rider
- Git
- Git LFS

## 현재 개발 범위

현재 저장소에는 프로필, 월드맵, 미니게임 선택, 난이도, 공통 리듬 판정, 점수, 옵션, 오디오 동기화와 지연 보정 시스템이 구현되어 있습니다.

스토리 뷰어, 단계별 튜토리얼, 해금·보상·스토리 진행 시스템 일부는 후속 확장 영역으로 남아 있습니다.

## 팀 리듬피플

| 이름 | 역할 |
| --- | --- |
| 임영택 | 팀장 |
| 주철민 | 부팀장 |
| 김동주 | PM |
| 정야후 | Git 관리 |
| 김영민 | QA |
| 유주연 | 발표 / 비주얼 에셋 |
| 조수경 | 총무 |
| 김상훈 | 개발 |
| 한준희 | 개발 |

팀원별 담당 기능, 기술적 의사결정과 프로젝트 회고는 [프로젝트 브로셔](https://www.notion.so/teamsparta/7-6-Part-Time-Beat-3792dc3ef514808d9bced8a2b6916207)에서 확인할 수 있습니다.

## 참고 링크

- [게임 플레이 영상](https://youtu.be/5rShTf2_pOs?si=eAFhhQbZcBBJ-dPV)
- [실행 빌드](https://drive.google.com/file/d/1pALNzBV3WtzKl0bubRrCOdZph3jbSF6U/view?usp=sharing)
- [프로젝트 브로셔](https://www.notion.so/teamsparta/7-6-Part-Time-Beat-3792dc3ef514808d9bced8a2b6916207)
- [Unreal Engine Documentation](https://dev.epicgames.com/documentation/ko-kr/unreal-engine/)
- [Audiokinetic Wwise](https://www.audiokinetic.com/ko/wwise/overview/)
