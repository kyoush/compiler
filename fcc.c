#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの型を表す値
enum {
      TK_NUM = 256, // 整数トークン
      TK_EOF,       // 入力の終わりを表すトークン
};

// トークンの型
typedef struct {
    int ty;         // トークンの型
    int val ;       // tyがTK_NUMの場合、その数値
    char *input;    // トークン文字列(エラーメッセージ用)
} Token;

// トークナイズした結果のトークン列はこの配列に保存する
// 100個以上のトークンは来ないとする
Token tokens[100];

// pが指している文字列をトークンに分割してtokensに保存する
void tokenize(char *p) {
    int i = 0;
    while (*p) {
        //空白文字をスキップ
        if (isspace(*p)) {
            p++;
            continue;
        }

        if (*p == '+' || *p == '-') {
            tokens[i].ty = *p;
            tokens[i].input = p;
            i++;
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens[i].ty = TK_NUM;
            tokens[i].input = p;
            tokens[i].val = strtol(p, &p, 10);
            i++;
            continue;
        }

        fprintf(stderr, "トークナイズできません: %s\n", p);
        exit(1);
    }

    tokens[i].ty = TK_EOF;
    tokens[i].input = p;
}

// エラーを報告するための関数
void error(int i) {
    fprintf(stderr, "予期しないトークンです: %s\n",
            tokens[i].input);
    exit(1);
}

// 抽象構文木のノードの型の定義
enum {
      ND_NUM = 256,   // 整数ノードの型
};

typedef struct Node {
    int ty;           // 演算子かND_NUM
    struct Node *lhs; // 左辺(left-hand side)
    struct Node *rhs; // 右辺(right-hand side)
    int val;          // tyがND_NUMの場合のみ使う
} Node;

// 新しいノードを作成する関数
Node *new_node(int ty, Node *lhs, Node *rhs) {
    Node *node = malloc(sizeof(Node));
    node->ty = ty;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

Node *new_node_num(int val) {
    Node *node = malloc(sizeof(Node));
    node->ty = ND_NUM;
    node->val = val;
    return node;
}

// 次のトークンが期待したかどうかをチェックする関数
// その場合のみ入力を1トークン進めて真を返す
int consume(int ty) {
    if (tokens[pos].ty != ty)
        return 0;
    pos++;
    return 1;
}

// パーサ
// add関数
Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume('+'))
            node =new_node('+', node, mul());
        else if (consume('-'))
            node = new_node('-', node, mul());
        else
            return node;
    }
}

// mul関数
Node *mul() {
    Node *node = term();

    for (;;) {
        if (consume('*'))
            node = new_node('*', node, term());
        else if (consume('/'))
            node = new_node('/', node, term());
        else
            return node;
    }
}

// term関数
Node *term() {
    if (consume('(')) {
        Node *node = add();
        if(!consume(')'))
            error("開きカッコに対応する閉じカッコがありません: %s",
                  tokens[pos].input);
        return node;
    }

    if (tokens[pos].ty == TK_NUM)
        return new_node_num(tokens[pos++].val);

    error("数値でも開きカッコでもないトークンです: %s",
          tokens[pos].input);
}


int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "引数の数が正しくありません\n");
        return 1;
    }

    // トークナイズする
    tokenize(argv[1]);
    
    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".global main\n");
    printf("main:\n");

    // 式の最初は数でなければならないので、それをチェックして最初のmov命令を出力
    if (tokens[0].ty != TK_NUM)
        error(0);
    printf("  mov rax, %d\n", tokens[0].val);
    
    // '+ <数>'あるいは'- <数>'というトークンの並びを消費しつつアセンブリを出力
    int i = 1;
    while (tokens[i].ty != TK_EOF) {
        if (tokens[i].ty == '+') {
            i++;
            if (tokens[i].ty != TK_NUM)
                error(i);
            printf("  add rax, %d\n", tokens[i].val);
            i++;
            continue;
        }

        if (tokens[i].ty == '-') {
            i++;
            if(tokens[i].ty != TK_NUM)
                error(i);
            printf("  sub rax, %d\n", tokens[i].val);
            i++;
            continue;
        }
        error(i);
    }
    
    printf("  ret\n");
    return 0;
}
