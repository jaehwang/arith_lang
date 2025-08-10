# 커밋 메시지 표준 템플릿 (arith_lang 프로젝트)

## 기본 구조
```
<type>: <subject>

<body>
- 변경사항 설명
- 기술적 세부사항
- 호환성 정보
- 이슈 참조: Fixes #123, Closes #456

<footer>
```

## Type 분류
- **feat**: 새로운 기능 추가
- **fix**: 버그 수정  
- **refactor**: 코드 리팩토링
- **docs**: 문서 변경
- **test**: 테스트 추가/수정
- **chore**: 빌드, 설정 변경
- **perf**: 성능 개선

## Subject 작성 규칙
- 50자 이내로 간결하게
- 동사원형으로 시작 (implement, add, fix, update 등)
- 첫 글자 소문자
- 마침표 없음
- 무엇을 했는지 명확히 표현

## Body 작성 가이드
- 변경사항을 bullet point(-)로 나열
- 각 포인트는 구체적이고 명확하게
- 기술적 세부사항 포함
- 호환성 정보 명시
- 동작 방식 설명

## Footer 규칙
- 도구 생성 정보: `🤖 Generated with [도구명](링크)`
- 공동 작성자: `Co-Authored-By: 이름 <이메일>`

## Body에서 이슈 참조 작성법
- `Fixes #123`: 버그 수정 시 사용
- `Closes #456`: 기능 구현 완료 시 사용  
- `Resolves #789`: 일반적인 이슈 해결 시 사용
- `Addresses #012`: 부분적 해결 시 사용
- 여러 이슈: `Fixes #42, Closes #38, Resolves #55`

## 프로젝트별 키워드
arith_lang 프로젝트에서 자주 사용할 수 있는 subject 키워드:
- `implement parser/lexer/evaluator`
- `add operator support`
- `fix precedence handling`
- `update CLI behavior`
- `enhance error messages`
- `refactor expression handling`

## 도구 정보
많이 쓰는 AI Coding 도구:
- 🤖 Generated with [Claude Code](https://claude.ai/code)
- 🤖 Generated with [GitHub Copilot](https://github.com/features/copilot)
- 🤖 Generated with [Cline](https://cline.ai)

이상의 목록에 없는 경우에는 사람에게 물어 보자.

## 실제 커밋 메시지 예시
```
feat: implement gcc-style default output behavior

- Modified parseCommandLine() to support optional -o flag with a.ll default
- Updated printUsage() to show both usage patterns with clear examples
- Maintains full backward compatibility with existing -o usage
- Creates a.ll in current directory when -o not specified (matches gcc a.out behavior)
- Updated memory bank documentation to reflect CLI improvements

🤖 Generated with [Claude Code](https://claude.ai/code)

Co-Authored-By: Claude <noreply@anthropic.com>
```