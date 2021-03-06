< TCP 채팅 프로그램 >

1. 서버를 실행시킨다. 서버의 아이디는 “root”, 비밀번호는 “1234”로 정했기 때문에 id, pw를 순서대로 입력해 서버에 해당하는 코드를 실행시킨다.

2. 클라이언트를 실행시킨다. 
- 아이디와 비밀번호를 임의로 입력하면 클라이언트로 접속된다.
- 서버의 아이디에 해당하는 “root”나 login.txt 파일에 이미 저장되어있는 아이디, 혹은 아이디가 너무 길 경우에는 사용할 수 없다. 
  이 경우 새로운 아이디로 다시 접속을 시도해야 한다.
- 접속한 클라이언트는 최소 5명이 되어야 채팅을 시작할 수 있으므로 다음 클라이언트를 기다린다. 5명이 되기 전까지는 채팅창에 글을 써도 전송되지 않는다.

3. 채팅을 시작한다.
- 최소 인원을 충족하면 모든 채팅창에 채팅할 수 있음을 알리는 문구가 출력된다.
- 대화 시 각 클라이언트의 창에 메시지를 전송한 클라이언트의 아이디와 메시지가 출력된다.
- 대화 시 서버의 창에 메시지를 전송한 클라이언트의 아이디와 메시지, 전송 시간이 출력된다.

4. "exit" 문구를 입력하여 채팅창을 빠져 나올 수 있다.
- 남은 인원이 5명 미만일 경우 채팅을 진행할 수 없기에 최소 인원을 만족할 때까지 기다려야한다. 마찬가지로 이 때 전송한 메시지는 다른 채팅창에 전달되지 않는다.
- 새로운 사람이 접속하여 최소 인원을 만족하면 다시 메시지를 주고받을 수 있다.

5. 채팅 종료
- 모든 대화는 record.txt 파일에 기록되어 있다.
- record.txt에는 모든 클라이언트의 대화 내용(아이디, 메시지, 전송 시간)이 저장되어 있다. 물론 메시지는 최소 인원이 5명이었던 상황의 채팅 내용만 해당한다.