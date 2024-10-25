#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <string.h>
#include <limits.h>

#define MAX_IDENTIFIER_SIZE 30
#define MAX_LINE_LENGTH 100
#define MAX_SIZE 100

char* readStringFromFile(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        printf("Dosya acilamadi: %s\n", filename);
        return NULL;
    }

    char* string = NULL;
    int size = 0;
    int length = 0;
    int ch;

    while ((ch = fgetc(file)) != EOF) {
        if (length >= size) {
            size = size == 0 ? 1 : size * 2;
            char* temp = realloc(string, size * sizeof(char));
            if (temp == NULL) {
                printf("Bellek tahsis edilemedi.\n");
                free(string);
                return NULL;
            }
            string = temp;
        }

        string[length++] = ch;
    }

    if (length >= size) {
        size++;
        char* temp = realloc(string, size * sizeof(char));
        if (temp == NULL) {
            printf("Bellek tahsis edilemedi.\n");
            free(string);
            return NULL;
        }
        string = temp;
    }

    string[length] = '\0';

    fclose(file);
    return string;
}

void writeStringToFile(const char* myText, const char* filename) {
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Dosya olusturulamadi: %s\n", filename);
        return;
    }

    fprintf(file, "%s", myText);

    fclose(file);
}

bool isOperator(char ch){// Function to check if a character is an operator
    if(ch == '+' || ch == '-'){
        return true;
    } else {
        return false;
    }
}

bool isKeyword(char* ch) { // Function to check if a string is a keyword
    if (!strcmp(ch, "new") || !strcmp(ch, "int") || !strcmp(ch, "text") ||
        !strcmp(ch, "size") || !strcmp(ch, "subs") || !strcmp(ch, "locate") ||
        !strcmp(ch, "insert") || !strcmp(ch, "override") || !strcmp(ch, "read") ||
        !strcmp(ch, "write") || !strcmp(ch, "from") || !strcmp(ch, "to") ||
        !strcmp(ch, "input") || !strcmp(ch, "output") || !strcmp(ch, "asText") ||
        !strcmp(ch, "asString")) {
        return true;
    } else {
        return false;
    }
}
char* toLower(char* ch){ // Function to convert a string to lowercase
    int length = strlen(ch);

    for(int i = 0; i < length; i++){
        ch[i] = tolower(ch[i]);
    }
    return ch;
}

int lexicalAnalyzer(char* filename){
    char c1, c2;
    char top[3];
    top[2] = '\0';
    FILE * psi = fopen(filename, "r"); //Reading .psi file

    if(psi == NULL){ //Checking the document
        printf("File Could Not Be Opened!");
        return 1;
    }
    FILE * lex = fopen("code.lex", "w"); // Reading .lex file
    c1 = fgetc(psi);
    while((c2= fgetc(psi)) != EOF){
        top[0] = c1;
        top[1] = c2;
        if(!strcmp(top, "/*")){
            c1=c2;
            while((c2= fgetc(psi)) != EOF){
                top[0] = c1;
                top[1] = c2;
                if(!strcmp(top,"*/")) {
                    break;
                }
                c1=c2;
            }
            if(c2==EOF){
                printf("Lexical Error");
            }
        }

        if(c1 == '"'){
            c1 = c2;
            fprintf(lex,"StringConstant(");
            while (c1 != '"'){
                if(c1 == EOF){
                    printf("string constant cannot terminate! \n");
                    break;
                }
                fprintf(lex, "%c", c1);
                c1 = fgetc(psi);
            }
            fprintf(lex,")\n");
            c2= fgetc(psi);
        }

        if(isOperator(c1)){
            fprintf(lex, "Operator(%c)\n", c1);
        }else if(!strcmp(top, ":=")){
            fprintf(lex,"Operator(:=)\n");
        }

        if(isalpha(c1)){
            char identifier[MAX_IDENTIFIER_SIZE + 1];
            int id_lenght = 0;
            identifier[id_lenght++] = c1;
            identifier[id_lenght++] = c2;
            while(isalnum(c1 = fgetc(psi)) || c1 == '_'){
                if(id_lenght < MAX_IDENTIFIER_SIZE){
                    identifier[id_lenght++] = c1;
                }else{
                    printf("Maximum identifier size is 30 digits\n");
                    while((isalnum(c1 = fgetc(psi)) || c1 == '_')){
                        id_lenght = 0;
                    }
                }
            }
            identifier[id_lenght] = '\0';
            if(isKeyword(identifier)){
                fprintf(lex,"Keyword(%s)\n", toLower(identifier));
            }else if(identifier[0] != '\n'){
                fprintf(lex,"Identifier(%s)\n", toLower(identifier));
            }
            c2= c1;
        }//if(isalpha(c1))
        if(isdigit(c1)){
            char intconstant[UINT_MAX + 1];
            int lenght = 0;
            intconstant[lenght++] = c1;
            if(isdigit(c2)) {
                intconstant[lenght++] = c2;
                while (isdigit(c1 = fgetc(psi))) {
                    if (lenght < UINT_MAX) {
                        intconstant[lenght++] = c1;
                    } else {
                        printf("Maximum integer size error!");
                        while (isdigit(c1 = fgetc(psi))) {
                            lenght = 0;
                        }
                    }
                }
                c2=c1;
            }
            intconstant[lenght] = '\0';
            if(intconstant[0] != '\0')
                fprintf(lex,"IntConst(%s)\n",intconstant);

        }//if(isdigit(c1))

        if(c2==';'){
            fprintf(lex,"EndOfLine \n");
        }


        c1=c2;
    }//while((c1= fgetc(psi)) != EOF)
    fclose(psi);
    fclose(lex);
}

typedef enum {
    TOKEN_ENDOFLINE,
    TOKEN_IDENTIFIER,
    TOKEN_INTCONST,
    TOKEN_OPERATOR,
    TOKEN_STRINGCONST,
    TOKEN_KEYWORD
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_LINE_LENGTH];
} Token;

Token lexToken(const char* line) {
    Token token;
    int i = 0;

    // Boşlukları atla
    while (line[i] == ' ') {
        i++;
    }

    // EndOfLine kontrolü
    if (strncmp(&line[i], "EndOfLine", 9) == 0) {
        token.type = TOKEN_ENDOFLINE;
        strcpy(token.value, "EndOfLine");
        return token;
    }

    // Identifier kontrolü
    if (strncmp(&line[i], "Identifier(", 11) == 0) {
        token.type = TOKEN_IDENTIFIER;
        sscanf(&line[i], "Identifier(%[^)])", token.value);
        return token;
    }

    // IntConst kontrolü
    if (strncmp(&line[i], "IntConst(", 9) == 0) {
        token.type = TOKEN_INTCONST;
        sscanf(&line[i], "IntConst(%[^)])", token.value);
        return token;
    }

    // Operator kontrolü
    if (strncmp(&line[i], "Operator(", 9) == 0) {
        token.type = TOKEN_OPERATOR;
        sscanf(&line[i], "Operator(%[^)])", token.value);
        return token;
    }

    if (strncmp(&line[i], "StringConstant(", 15) == 0) {
        token.type = TOKEN_STRINGCONST;
        sscanf(&line[i], "StringConstant(%[^)])", token.value);
        return token;
    }

    // Keyword kontrolü
    if (strncmp(&line[i], "Keyword(", 8) == 0) {
        token.type = TOKEN_KEYWORD;
        sscanf(&line[i], "Keyword(%[^)])", token.value);
        return token;
    }
    // Bilinmeyen bir token varsa hata ver
    fprintf(stderr, "Geçersiz token: %s\n", line);
    exit(1);
}



Token tokenlist[100];
int isize = 0;

int lexer(){
    FILE* file = fopen("code.lex", "r");
    if (file == NULL) {
        fprintf(stderr, "Dosya açılamadı\n");
        return 1;
    }
    char line[MAX_LINE_LENGTH];
    while (fgets(line, 999, file) != NULL) {
        Token token = lexToken(line);
        tokenlist[isize++] = token;
    }
    fclose(file);
    return 0;
}

typedef struct{
    char key[50];
    union {
        char strValue[50];
        int intValue;
    } value;
    int isString;
}DictionaryEntry;

typedef struct{
    DictionaryEntry entries[MAX_SIZE];
    int count;
}Dictionary;


void addStringToDictionary(Dictionary* dictionary, const char* key, const char* value) {
    if (dictionary->count >= MAX_SIZE) {
        printf("Sözlük dolu, yeni öğe eklenemiyor!\n");
        return;
    }
    DictionaryEntry newEntry;
    strcpy(newEntry.key, key);
    strcpy(newEntry.value.strValue, value);
    newEntry.isString = 1;

    dictionary->entries[dictionary->count] = newEntry;
    dictionary->count++;
}

void addIntToDictionary(Dictionary* dictionary, const char* key, int value) {
    if (dictionary->count >= MAX_SIZE) {
        printf("Sözlük dolu, yeni öğe eklenemiyor!\n");
        return;
    }

    DictionaryEntry newEntry;
    strcpy(newEntry.key, key);
    newEntry.value.intValue = value;
    newEntry.isString = 0;

    dictionary->entries[dictionary->count] = newEntry;
    dictionary->count++;
}

// Sözlükte bir anahtarın değerini değiştirme (string değer)
void setStringInDictionary(Dictionary* dictionary, const char* key, const char* value) {
    for (int i = 0; i < dictionary->count; i++) {
        if (strcmp(dictionary->entries[i].key, key) == 0 && dictionary->entries[i].isString) {
            strcpy(dictionary->entries[i].value.strValue, value);
            return;
        }
    }

    printf("Anahtar bulunamadı veya değer bir string değil!\n");
}

// Sözlükte bir anahtarın değerini değiştirme (int değer)
void setIntInDictionary(Dictionary* dictionary, const char* key, int value) {
    for (int i = 0; i < dictionary->count; i++) {
        if (strcmp(dictionary->entries[i].key, key) == 0 && !dictionary->entries[i].isString) {
            dictionary->entries[i].value.intValue = value;
            return;
        }
    }

    printf("Anahtar bulunamadı veya değer bir int değil!\n");
}

// Sözlükte bir anahtarın değerini döndürme
void getValueFromDictionary(const Dictionary* dictionary, const char* key) {
    for (int i = 0; i < dictionary->count; i++) {
        if (strcmp(dictionary->entries[i].key, key) == 0) {
            if (dictionary->entries[i].isString) {
                printf("%s\n", dictionary->entries[i].value.strValue);
            } else {
                printf("%d\n", dictionary->entries[i].value.intValue);
            }
            return;
        }
    }

    printf("Anahtar bulunamadı!\n");
}

int getTypeFromDictionary(const Dictionary* dictionary, const char* key) {
    for (int i = 0; i < dictionary->count; i++) {
        if (strcmp(dictionary->entries[i].key, key) == 0) {
            if (dictionary->entries[i].isString) {
                return 0;
            } else {
                return 1;
            }
        }
    }

    printf("Anahtar bulunamadı!\n");
    return -1;
}

int getIntValueFromDictionary(const Dictionary* dictionary, const char* key) {
    for (int i = 0; i < dictionary->count; i++) {
        if (strcmp(dictionary->entries[i].key, key) == 0) {
            if (dictionary->entries[i].isString) {
                printf("Yanlış tip çağırımı!");
                return -1;
            } else {
                return  dictionary->entries[i].value.intValue;
            }
        }
    }
    printf("Anahtar bulunamadı!\n");
    return -1;
}

const char* getStringValueFromDictionary(const Dictionary* dictionary, const char* key) {
    for (int i = 0; i < dictionary->count; i++) {
        if (strcmp(dictionary->entries[i].key, key) == 0) {
            if (dictionary->entries[i].isString) {
                return dictionary->entries[i].value.strValue;
            } else {
                printf("Yanlış tip çağırımı!");
                return NULL;
            }
        }
    }
    printf("Anahtar bulunamadı!\n");
    return NULL;
}

void altStringiCikar(char* hedef, const char* altString) {
    int hedefUzunluk = strlen(hedef);
    int altStringUzunluk = strlen(altString);
    int i, j, k;

    for (i = 0; i < hedefUzunluk; i++) {
        // Eşleşme bulmak için alt-stringi kontrol et
        if (hedef[i] == altString[0]) {
            // Eşleşme bulundu, alt-stringin tamamının eşleşip eşleşmediğini kontrol et
            for (j = 1, k = i + 1; j < altStringUzunluk && k < hedefUzunluk; j++, k++) {
                if (hedef[k] != altString[j]) {
                    break;
                }
            }
            // Alt-string tamamıyla eşleşiyorsa, ilgili karakterleri çıkar
            if (j == altStringUzunluk) {
                for (k = i, j = i + altStringUzunluk; j <= hedefUzunluk; k++, j++) {
                    hedef[k] = hedef[j];
                }
                break;
            }
        }
    }
}


int arithmeticInt(Dictionary* dictionary, const char* key ,int resul, Token sign, Token second ){
    int result;
    int firstValue = resul;
    int secondValue = atoi(second.value);
    if(!strcmp(sign.value, "+")){
        result = firstValue + secondValue;
    }else if(!strcmp(sign.value, "-") && firstValue >= secondValue){
        result = firstValue - secondValue;
    }else{
        printf("Hatalı bir işlem yaptınız!(1)");
        result = 0;
    }
    return result;
}

const char* arithmeticString(Dictionary* dictionary, const char* key, const char* resul, Token sign, const char* second){
    const char* result;
    if(!strcmp(sign.value, "+")){
        result = strcat(resul,second);
    }else if(!strcmp(sign.value, "-") && strlen(resul) >= strlen(second)){
        altStringiCikar(resul,second);
        result = resul;
    }else{
        printf("Hatalı bir işlem yaptınız!(2)");
        result = "";
    }
    return result;
}

int locate(const char *bigText, const char *smallText, int start) {
    int bigLen = strlen(bigText);
    int smallLen = strlen(smallText);

    if (start < 0 || start >= bigLen)
        return 0;

    int i, j;
    for (i = start; i <= bigLen - smallLen; i++) {
        j = 0;
        while (j < smallLen && bigText[i + j] == smallText[j])
            j++;

        if (j == smallLen)
            return i;
    }

    return 0;
}

char* insert(const char *myText, int location, const char *insertText) {
    int myLen = strlen(myText);
    int insertLen = strlen(insertText);
    int resultLen = myLen + insertLen;

    char *result = (char*)malloc((resultLen + 1) * sizeof(char));

    if (location < 0 || location > myLen) {
        strcpy(result, myText);
        return result;
    }

    strncpy(result, myText, location);
    strncpy(result + location, insertText, insertLen);
    strcpy(result + location + insertLen, myText + location);

    return result;
}

char* override(char* myText, int location, const char* ovrText) {
    int ovrLength = strlen(ovrText);
    int textLength = strlen(myText);

    if (location < 0 || location >= textLength)
        return myText; // Geçersiz konum, değişiklik yapmadan myText'i döndür

    // Determine the length of the remaining portion of myText
    int remainingLength = textLength - location;

    // Calculate the number of characters to copy
    int copyLength = (ovrLength <= remainingLength) ? ovrLength : remainingLength;

    // Allocate memory for the new string
    char* result = malloc((textLength + 1) * sizeof(char));

    // Copy the original string to the result
    strcpy(result, myText);

    // Copy ovrText to the result starting from the specified location
    strncpy(result + location, ovrText, copyLength);

    // Add a null terminator to ensure proper string termination
    result[textLength] = '\0';

    return result;
}

void assignment(Dictionary* dictionary, Token* is){
    int i = 3;
    if(getTypeFromDictionary(dictionary, is[0].value) == 0){
        const char *result;
        if(is[2].type == TOKEN_STRINGCONST) {
            result = is[2].value;
        }else if(is[i-  1].type == TOKEN_IDENTIFIER) {
            const char *temp = is[i - 1].value;
            result = strcat(result, temp);
        }
        while(is[i].type != TOKEN_ENDOFLINE){
            if(is[i + 1].type == TOKEN_IDENTIFIER){
                result = arithmeticString(dictionary, is[0].value, result, is[i], getStringValueFromDictionary(dictionary,is[i+1].value));
                i += 2;
            }else if(is[i + 1].type == TOKEN_STRINGCONST ) {
                result = arithmeticString(dictionary, is[0].value, result, is[i], is[i + 1].value);
                i += 2;
            }
        }
        setStringInDictionary(dictionary, is[0].value, result);
    }else if(getTypeFromDictionary(dictionary, is[0].value) == 1) {
        int result;
        if(is[2].type == TOKEN_INTCONST) {
            result = atoi(is[2].value);
        }
        while(is[i].type != TOKEN_ENDOFLINE){
            if(is[i - 1].type == TOKEN_IDENTIFIER){
                int temp = getIntValueFromDictionary(dictionary, is[i - 1].value);
                result += temp;
            }
            result = arithmeticInt(dictionary, is[0].value, result, is[i], is[i + 1]);
            i += 2;
        }
        if(is[i - 1].type == TOKEN_IDENTIFIER) {
            int temp = getIntValueFromDictionary(dictionary, is[i - 1].value);
            result += temp;
        }
        setIntInDictionary(dictionary, is[0].value, result);
    }
}

void identifier(Dictionary* dictionary, Token* is){
    if(!strcmp(is[0].value, "new")){
        if(!strcmp(is[1].value, "text")){
            if(is[2].type == TOKEN_IDENTIFIER){
                addStringToDictionary(dictionary, is[2].value, "");
            }
        }else if(!strcmp(is[1].value, "int")){
            if(is[2].type == TOKEN_IDENTIFIER){
                addIntToDictionary(dictionary, is[2].value, -1);
            }
        }
    }
    else if(!strcmp(is[0].value, "size")){
        const char* output = getStringValueFromDictionary(dictionary, is[1].value);
        printf("%d\n", strlen(output));
    }
    else if(!strcmp(is[0].value, "subs")){
        int begin = atoi(is[2].value);
        int end = atoi(is[3].value);
        const char* myText = getStringValueFromDictionary(dictionary, is[1].value);
        if (begin < 0 || end < 0 || begin > end || strlen(myText) < end) {
            printf("Hataaaa\n");  // Hatalı giriş durumunda NULL döndürülür.
        }else {
            int subStringLength = end - begin + 1;
            char *substring = (char *) malloc(subStringLength + 1);  // Alt dize için bellek tahsisi

            if (substring == NULL) {
                printf("Hata1!\n");  // Bellek tahsisi başarısız olduysa NULL döndürülür.
            }

            strncpy(substring, myText + begin, subStringLength);
            substring[subStringLength] = '\0';  // Alt dizenin sonuna NULL karakteri eklenir.

            printf("%s", substring);
        }
    }
    else if(!strcmp(is[0].value, "locate")){
        const char* bigText;
        const char* smallText;
        if(is[1].type == TOKEN_IDENTIFIER){
            bigText = getStringValueFromDictionary(dictionary, is[1].value);
        }else if(is[1].type == TOKEN_STRINGCONST){
            bigText = is[1].value;
        }
        if(is[2].type == TOKEN_IDENTIFIER){
            smallText = getStringValueFromDictionary(dictionary, is[2].value);
        }else if(is[2].type == TOKEN_STRINGCONST){
            smallText = is[2].value;
        }
        int location = locate(bigText, smallText, atoi(is[3].value));
        printf("%d\n", location);
    }
    else if(!strcmp(is[0].value, "insert")){
        const char* myText = getStringValueFromDictionary(dictionary,is[1].value);
        const char* insertText;
        if(is[3].type == TOKEN_IDENTIFIER){
            insertText = getStringValueFromDictionary(dictionary, is[3].value);
        }else if(is[3].type == TOKEN_STRINGCONST){
            insertText = is[3].value;
        }
        char* result = insert(myText, atoi(is[2].value), insertText);
        printf("%s\n", result);
        setStringInDictionary(dictionary, is[1].value, result);
    }
    else if(!strcmp(is[0].value, "override")){
        const char* myText = getStringValueFromDictionary(dictionary,is[1].value);
        const char* ovrText;
        if(is[3].type == TOKEN_IDENTIFIER){
            ovrText = getStringValueFromDictionary(dictionary, is[3].value);
        }else if(is[3].type == TOKEN_STRINGCONST){
            ovrText = is[3].value;
        }
        char* result = override(myText, atoi(is[2].value), ovrText);
        printf("%s\n", result);
        setStringInDictionary(dictionary, is[1].value, result);

    }
    else if(!strcmp(is[0].value, "asstring")){
        char* strin;
        sprintf(strin, "%d", getIntValueFromDictionary(dictionary, is[1].value));
        printf("%s\n", strin);
    }
    else if(!strcmp(is[0].value, "astext")){
        int number = atoi(getStringValueFromDictionary(dictionary ,is[1].value));
        printf("%d\n", number);
    }
    else if(!strcmp(is[0].value, "read")){
        const char* readname = is[1].value;
        char* readstring = readStringFromFile(is[3].value);
        setStringInDictionary(dictionary, readname, readname);
    }
    else if (!strcmp(is[0].value, "write")){//write myText to yourTextFile;
        writeStringToFile(is[1].value, is[3].value);
    }
    else if(!strcmp(is[0].value, "input")){
        const char* key = is[1].value;
        char* value = is[3].value;
        setStringInDictionary(dictionary,key,value);
    }
    else if(!strcmp(is[0].value, "output")){
        if(is[1].type==TOKEN_IDENTIFIER) {
            getValueFromDictionary(dictionary, is[1].value);
        }
    }
}

void islem(Dictionary* dictionary, Token* is){
    int i = 0;
    if(is[0].type == TOKEN_KEYWORD){
        identifier(dictionary ,is);
    }else if(is[1].type == TOKEN_OPERATOR && !strcmp(is[1].value, ":=")){
        assignment(dictionary, is);
    }
}

void interpret(Dictionary* dictionary){
    Token newtok[20];
    int a = 0;
    for(int i = 0 ; i< isize; i++){

        if(tokenlist[i].type == TOKEN_ENDOFLINE ){
            newtok[a++] = tokenlist[i];
            islem(dictionary,newtok);
            a = 0;
        }else{
            newtok[a++] = tokenlist[i];
        }
    }
}
void temizle() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

int main() {
    Dictionary myDictionary;
    myDictionary.count = 0;
    char dosyaadi[100];
    printf("Dosyanin adini '.pj' uzantili sekilde giriniz: ");
    scanf("%s", dosyaadi);
    lexicalAnalyzer(dosyaadi);
    lexer();
    interpret(&myDictionary);
    int sayi;
    printf("Kapatmak icin bir tusa basin. ");
    scanf("%d", &sayi);
    return 0;
}
