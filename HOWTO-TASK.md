# HOWTO: AI 도구 협업으로 대규모 작업(Task) 수행하기

이 문서의 목적은 AI를 이용한 개발에서 큰 Task를 Sub Task로 나눠 진행하기 위한 효과적인 방법론을 제시하는 것입니다. `specs/variables.md`, `plans/mut-implementation-plan.md` 등의 파일을 사례로 이 방법론을 소개합니다. Sub Task 간의 '의도 이탈(drift)' 문제를 해결하기 위해 명세(spec)를 '단일한 진실의 원천(Single Source of Truth)'으로 삼고 각 작업 단계마다 이를 기준으로 일관성을 유지하는 것이 핵심입니다.

## 1) 배경: Context Engineering의 필요성과 '의도 이탈' 문제

AI 도구를 활용한 협업 개발에서는 각 도구가 작업 맥락(context)을 어떻게 해석하는지에 따라 결과가 달라질 수 있습니다. 여러 도구와 단계가 얽힌 대규모 작업에서는, 맥락 전달이 불충분하거나 일관되지 않으면 초기 의도에서 벗어나는 '의도 이탈(drift)' 문제가 빈번하게 발생합니다. 예를 들어, `plans/mut-implementation-plan.md`와 같은 계획 문서만을 기준으로 삼을 경우, 도구별 해석 차이로 인해 최종 산출물이 원본 명세(`specs/variables.md`)와 달라질 위험이 있습니다.

이러한 문제를 해결하기 위해서는, **Context Engineering**—즉, 작업의 기준이 되는 명세와 현재 맥락을 모든 도구와 단계에 명확히 주입하고, 지속적으로 동기화하는 체계적인 워크플로가 필수적입니다. 본 가이드는 원본 명세를 단일한 진실의 원천(SoT)으로 삼고, 각 단계마다 명세와 맥락을 반복적으로 주입·검증하는 방법론을 제시합니다.

---

## 2) 핵심 원칙

1.  **단일 진실의 원천(SoT)**: 원래 명세 파일(specs/variables.md)을 최우선 참조한다.
2.  **계획은 보조물**: `plans/mut-implementation-plan.md`는 구현 순서와 커버리지 체크 용도로 사용하되, 충돌 시 spec이 우선한다.
3.  **단계 경계 = 커밋 경계**: 각 sub task가 끝날 때마다 커밋한다. 커밋 메시지에는 해당 작업이 참조한 spec, 계획 단계, 테스트 범위를 명확히 기록한다.
5.  **지속적 드리프트 감지**: 테스트 케이스는 사람이 직접 작성하거나 AI가 생성할 수 있다. 생성된 테스트 케이스가 명세(spec)에 부합하는지 반드시 사람이 리뷰하여 드리프트를 감지한다.
6.  **도구 및 Sub Task 전환 시 일관성 유지**: 도구 간 또는 Sub Task 간 전환에서는 AGENTS.md, rules/anchor-comments.md, rules/commit-log.md 등 프로젝트 규칙을 통해 일관성을 유지한다.
8.  **명시적 범위 관리**: 핵심 기능에 집중하고, 의존성이 있거나 후순위인 기능은 spec 문서의 "Implementation Status"나 "로드맵"에 명시하여 작업을 분리한다.

---

## 3) 전체 워크플로 개요

* 준비
  - SoT 지정: specs/variables.md. 요구 사항, 설계, 테스트 케이스 등을 포함한 명세 파일을 단일한 진실의 원천으로 삼는다.
  - 계획 수립: `plans/mut-implementation-plan.md`. Sub Task들을 도출하고 구현 순서를 작성한다.
  - 프로젝트 규칙 준수: AGENTS.md, rules/anchor-comments.md, rules/commit-log.md

* 실행 루프(각 단계 반복)
  1) Sub Task 구현/리팩토링
  2) 테스트 케이스 작성 (필요시)
  3) 자동화 실행(빌드, 테스트)
  4) 결과 검토 및 드리프트 감지/교정
  5) 커밋

* 종료
  - 회귀 테스트/린팅/포맷/문서화
  - 변경 로그/릴리스 노트에 spec 준수 항목 요약
  - specs/variables.md의 Implementation Status로 현재 범위 명시, 로드맵 항목 분리

---

## 4) 실전 사례

다음 두 파일은 모두 사용자의 간단한 입력을 바탕으로 AI가 생성한 산출물입니다. 이들은 명세(specs/variables.md)와 구현 계획(plans/mut-implementation-plan.md)을 기반으로 하여, AI 도구가 어떻게 협업하여 대규모 작업을 수행할 수 있는지를 보여줍니다.

### [variables.md](specs/variables.md) (명세)
  - 요구 사항
    - 기본 불변(immutable-by-default), `mut`로 명시적 가변 선언
    - 섀도잉 허용, 재할당은 타입 일치 강제
    - 정밀 진단(file:line:col, 소스 라인, 캐럿, note/help)과 미정의 변수 에러
  - 설계
    - Syntax: 선언/할당 BNF와 예제
    - Semantics: 불변/가변/섀도잉/스코프 규칙, 섀도잉 vs 뮤테이션 차이
    - 구현 가이드 핵심: Symbol(name, type, is_mutable, is_initialized), AssignmentExprAST 플래그, mut 파싱, 재할당 타입 검사/에러 형식
  - 테스트
    - 변수 불변성, 가변성, 섀도잉, 함수 파라미터, 스코프 등 주요 케이스별 테스트 예시와 기대 결과 포함
    - 에러 상황(재할당, 타입 불일치, 미정의 변수)에 대한 진단 메시지 형식 명확히 제시
    - 테스트 케이스는 명세의 요구 사항과 설계에 기반하여 작성됨

### [mut-implementation-plan.md](plans/mut-implementation-plan.md) (구현 계획)
  - 목적: 명세의 요구 사항을 충실히 구현하기 위한 변경 범위와 구체적 순서 정리
  - 변경 대상: 각 컴포넌트별로 mutability(가변성) 지원 추가 및 구조 변경 제안
  - 테스트 전략: 단위/통합 테스트 파일 구조와 주요 시나리오 명확히 제시
  - 구현 순서 요약:
    1. 변수 선언/할당 로직에 mut 플래그 추가
    2. Symbol 구조체에 is_mutable, is_initialized 필드 반영
    3. AssignmentExprAST에 mut 처리 및 타입 검사 로직 구현
    4. 섀도잉/재할당 규칙 적용 및 에러 메시지 형식 통일
    5. 테스트 케이스 작성 및 명세와 일치 여부 검증


## 5) 참고 

* [Cline의 Deep Planning](https://docs.cline.bot/features/slash-commands/deep-planning)