# AI Agent 사용하기

## 파일 구조 요약

```
arith_lang/
├── AGENTS.md                           # 메인 AI 에이전트 instruction 파일
├── CLAUDE.md                          # Claude Code 전용 프로젝트 가이드라인
├── .github/
│   ├── copilot-instructions.md        # GitHub Copilot instruction 파일
│   └── prompts/
│       └── deep-planning.prompt.md    # Deep Planning 프롬프트 템플릿
├── .clinerules/
│   └── cline-instructions.md          # Cline 전용 작업 지침
├── memory-bank/
│   ├── projectbrief.md                # 프로젝트 기본 요구사항과 목표
│   ├── productContext.md              # 프로젝트 존재 이유와 사용자 경험 목표
│   ├── activeContext.md               # 현재 작업 포커스와 최근 변경사항
│   ├── systemPatterns.md              # 시스템 아키텍처와 기술적 결정사항
│   ├── techContext.md                 # 사용 기술과 개발 환경 설정
│   └── progress.md                    # 현재 상태와 남은 작업
└── rules/
    ├── memory-bank.md                 # Memory Bank 시스템 지침
    ├── anchor-comments.md             # 코드 주석 가이드라인
    └── commit-log.md                  # 커밋 메시지 작성 규칙
```

## AI Agent Instructions

이 프로젝트는 다양한 AI 코딩 어시스턴트가 일관된 품질과 스타일로 작업할 수 있도록 종합적인 instruction 시스템을 제공합니다.

## AI Agent별 Instruction 파일들

- **GitHub Copilot**: `.github/copilot-instructions.md`
  - AGENTS.md 규칙을 따르도록 지시
  - 모든 파일(`**`)에 적용

- **Cline**: `.clinerules/cline-instructions.md`
  - Cline 전용 작업 지침
  - AGENTS.md 참조

- **Claude Code**: `CLAUDE.md`
  - Claude Code 전용 프로젝트 가이드라인
  - AGENTS.md 참조하여 모든 규칙 적용

- **Deep Planning**: `.github/prompts/deep-planning.prompt.md`
  - 복잡한 기능 구현 전 체계적 접근 방법론
  - 3단계 프로세스: Silent Investigation → Discussion → Implementation Plan
  - 코드베이스 탐사, 요구사항 명확화, 상세 계획 수립
  - `implementation_plan.md` 생성으로 구현 청사진 제공

## 핵심 통합 규칙 시스템

- **AGENTS.md**: 모든 AI 에이전트가 따라야 할 메인 instruction 파일
  - Memory Bank 시스템 지침 (`rules/memory-bank.md`)
  - 코드 주석 가이드라인 (`rules/anchor-comments.md`)  
  - 커밋 메시지 작성 규칙 (`rules/commit-log.md`)
  - 디렉토리별 AGENTS.md 파일 관리 지침

## Memory Bank 시스템

AI 에이전트의 세션 간 연속성을 보장하기 위한 문서화 시스템:

```
memory-bank/
├── projectbrief.md      # 프로젝트 기본 요구사항과 목표
├── productContext.md    # 프로젝트 존재 이유와 사용자 경험 목표
├── activeContext.md     # 현재 작업 포커스와 최근 변경사항
├── systemPatterns.md    # 시스템 아키텍처와 기술적 결정사항
├── techContext.md       # 사용 기술과 개발 환경 설정
└── progress.md          # 현재 상태와 남은 작업
```

## 코드 품질 규칙 시스템

- **Anchor Comments**: `rules/anchor-comments.md`에서 정의된 코드 주석 가이드라인
- **Commit Messages**: `rules/commit-log.md`에서 정의된 커밋 메시지 작성 규칙
- **Consistency**: 모든 AI 에이전트가 동일한 코드 스타일과 문서화 표준 준수

## AI Agent 사용 시 워크플로우

1. **세션 시작시**: 
   - `rules/memory-bank.md` 읽고 모든 Memory Bank 파일 확인
   - 해당 AI 에이전트별 instruction 파일 확인

2. **코드 작업시**: 
   - `rules/anchor-comments.md` 규칙에 따른 주석 작성
   - 디렉토리별 `AGENTS.md` 파일 확인 및 업데이트

3. **커밋 작업시**: 
   - `rules/commit-log.md` 가이드라인에 따른 메시지 작성

4. **문서화 작업시**:
   - Memory Bank 시스템을 통한 프로젝트 상태 유지

5. **복잡한 기능 구현시**:
   - `.github/prompts/deep-planning.prompt.md` 활용한 Deep Planning 수행
   - 체계적인 코드베이스 탐사와 요구사항 분석
   - 상세한 구현 계획서 작성 후 단계별 실행

## 장점

- **일관성**: 여러 AI 에이전트 간 동일한 코드 품질 보장
- **연속성**: Memory Bank를 통한 세션 간 컨텍스트 유지
- **확장성**: 새로운 AI 도구 추가시 AGENTS.md 참조만으로 통합 가능
- **유지보수성**: 중앙집중식 규칙 관리로 일관된 업데이트

이 시스템을 통해 GitHub Copilot, Cline, Claude Code 등 다양한 AI 도구가 협업하더라도 일관된 코드 품질과 문서화 표준을 유지할 수 있습니다.
