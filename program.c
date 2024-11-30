#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <openssl/sha.h>

#define MAX_WORDS 2048   
#define MAX_WORD_LENGTH 50 
#define WORDS_PER_LINE 8   
#define SELECTED_WORDS 11
#define WALLET_WORDS 12


char *bip39_words[MAX_WORDS];  
int word_count = 0;            
int selected_indices[SELECTED_WORDS];
int wallet_words[WALLET_WORDS]; 
int user_bits = 0;  

void load_words(const char *filename);
void calculate_max_lengths(int *max_index_length, int *max_word_length);
void show_all_words();
void search_words_by_string(const char *str);
void display_selected_words();
void select_11_words();
void initialize_selected_indices();
void free_memory();
void calculate_12th_word_index();
void display_12_words();
void clear_screen();
void entropy_seed();

void entropy_seed() {
    unsigned char data[(WALLET_WORDS * 11 + 7) / 8]; 
    char hex_output[(WALLET_WORDS * 11 + 7) / 8 * 2 + 1];

    memset(data, 0, sizeof(data));

    int bit_index = 0;
    for (int i = 0; i < WALLET_WORDS; i++) {
        int index = wallet_words[i];
        for (int j = 10; j >= 0; j--) {
            if (index & (1 << j)) {
                data[bit_index / 8] |= (1 << (7 - (bit_index % 8)));
            }
            bit_index++;
        }
    }

    for (int i = 0; i < sizeof(data); i++) {
        sprintf(hex_output + (i * 2), "%02x", data[i]);
    }
    hex_output[sizeof(data) * 2] = '\0'; 

    printf("Entropia (seed) em hexadecimal: %s\n", hex_output);
}


void clear_screen() {
    printf("\033[H\033[J");
}

void load_words(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Erro ao abrir arquivo de palavras");
        exit(EXIT_FAILURE);
    }

    char buffer[MAX_WORD_LENGTH];
    while (fgets(buffer, sizeof(buffer), file) != NULL) {
        buffer[strcspn(buffer, "\n")] = '\0';

        bip39_words[word_count] = malloc(strlen(buffer) + 1);
        if (bip39_words[word_count] == NULL) {
            perror("Erro de malloc");
            exit(EXIT_FAILURE);
        }
        strcpy(bip39_words[word_count], buffer);
        word_count++;

        if (word_count >= MAX_WORDS) {
            break;
        }
    }
    fclose(file);
}

void calculate_max_lengths(int *max_index_length, int *max_word_length) {
    *max_index_length = snprintf(NULL, 0, "%d", word_count - 1);
    *max_word_length = 0;
    for (int i = 0; i < word_count; i++) {
        int word_len = strlen(bip39_words[i]);
        if (word_len > *max_word_length) {
            *max_word_length = word_len;
        }
    }
}

void show_all_words() {
    int max_index_length, max_word_length;
    calculate_max_lengths(&max_index_length, &max_word_length);

    printf("Todas palavras:\n");

    for (int i = 0; i < word_count; i++) {
        printf("%-*d %-*s", max_index_length, i, max_word_length + 2, bip39_words[i]);

        if ((i + 1) % WORDS_PER_LINE == 0) {
            printf("\n");
        }
    }

    if (word_count % WORDS_PER_LINE != 0) {
        printf("\n");
    }
}

void search_words_by_string(const char *str) {
    int max_index_length, max_word_length;
    calculate_max_lengths(&max_index_length, &max_word_length);

    printf("Palavras iniciadas com '%s':\n", str);
    int count = 0; 
    size_t str_len = strlen(str);
    for (int i = 0; i < word_count; i++) {
        if (strncasecmp(bip39_words[i], str, str_len) == 0) {
            printf("%-*d %-*s", max_index_length, i, max_word_length + 2, bip39_words[i]);
            count++;

            if (count % WORDS_PER_LINE == 0) {
                printf("\n");
            }
        }
    }

    if (count % WORDS_PER_LINE != 0) {
        printf("\n");
    }
}

void display_selected_words() {
    printf("\nAs 12 palavras sao:\n");
    for (int i = 0; i < WALLET_WORDS; i++) {
        printf("%d-%s\n", wallet_words[i], bip39_words[wallet_words[i]]);
    }
}

void select_11_words() {
    int selected_count = 0;

    for (int i = 0; i < SELECTED_WORDS; i++) {
        if (selected_indices[i] != -1) {
            selected_count++;
        }
    }

    while (selected_count < SELECTED_WORDS) {

        int index;
        printf("Insira o indice da palavra(-1 para voltar, -0 para limpar): ");
        scanf("%d", &index);

        if (index == -1) {
            clear_screen();
            return;  
        } else if (index == -0) {
            clear_screen();
            initialize_selected_indices();
            selected_count = 0;
            printf("Selecao limpada.\n");
            continue;  
        }

        if (index >= 0 && index < word_count) {
            selected_indices[selected_count] = index;
            selected_count++;
        } else {
            printf("Indice invalido.\n");
        }

        if (selected_count > 0) {
            clear_screen();
            printf("Palavras selecionadas:\n");
            for (int i = 0; i < selected_count; i++) {
                printf("%d-%s\n", selected_indices[i], bip39_words[selected_indices[i]]);
            }
            if (selected_count < SELECTED_WORDS) {
            printf("Selecione mais %d palavras\n", SELECTED_WORDS - selected_count);
            }
        }
    }

    if (selected_count == SELECTED_WORDS && wallet_words[SELECTED_WORDS] == -1) {
    printf("Insira o numero de pre-cheksum(0-127): ");
    scanf("%d", &user_bits);
    calculate_12th_word_index();
    if (user_bits < 0 || user_bits > 127) {
        printf("Sequencia 7-bits invalida.\n");
        return;
    }
    }

    display_selected_words();
    entropy_seed();
}

void initialize_selected_indices() {
    for (int i = 0; i < SELECTED_WORDS; i++) {
        selected_indices[i] = -1;
    }
}

void initialize_wallet_words() {
    for (int i = 0; i < WALLET_WORDS; i++) {
        wallet_words[i] = -1;
    }
}

void free_memory() {
    for (int i = 0; i < word_count; i++) {
        free(bip39_words[i]);
    }
}

void calculate_12th_word_index() {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    unsigned char data[(SELECTED_WORDS * 11 + 7) / 8];

    memset(data, 0, sizeof(data));

    int bit_index = 0;
    for (int i = 0; i < SELECTED_WORDS; i++) {
        int index = selected_indices[i];
        for (int j = 10; j >= 0; j--) {
            if (index & (1 << j)) {
                data[bit_index / 8] |= (1 << (7 - (bit_index % 8)));
            }
            bit_index++;
        }
    }

    for (int j = 6; j >= 0; j--) {
        if (user_bits & (1 << j)) {
            data[bit_index / 8] |= (1 << (7 - (bit_index % 8)));
        }
        bit_index++;
    }

    SHA256(data, sizeof(data), hash);

    int checksum_bits = 4; 
    int checksum = hash[0] >> (8 - checksum_bits);

    int combined = (user_bits << checksum_bits) | checksum;

    int index_12th_word = combined % word_count;
        for (int i = 0; i < SELECTED_WORDS; i++) {
        wallet_words[i] = selected_indices[i];
    }
    wallet_words[SELECTED_WORDS] = index_12th_word;
}


int main() {
    load_words("portuguese.txt");
    initialize_selected_indices();
    initialize_wallet_words();
    clear_screen();

    int choice;
    char search_string[MAX_WORD_LENGTH];

    do {
        printf("\nBIP-39 Mnemonic Interface\n");
        printf("1. Mostrar todas palavras\n");
        printf("2. Pesquisar palavras por letra\n");
        if(wallet_words[SELECTED_WORDS] == -1) {
        printf("3. Selecionar 11 palavras interativamente\n");}else{
        printf("3. Regerar palavras\n");};
        printf("4. Sair\n");
        printf("Selecione: ");
        scanf("%d", &choice);

        switch (choice) {
            case 1:
                clear_screen();
                show_all_words();
                break;
            case 2:
                clear_screen();
                printf("Digite a letra inicial: ");
                scanf("%s", search_string); 
                search_words_by_string(search_string);
                break;
            case 3:
                clear_screen();
                if(wallet_words[SELECTED_WORDS] == -1) {
                select_11_words();}else{
                initialize_selected_indices();
                initialize_wallet_words();
                select_11_words();};
                break;
            case 4:
                clear_screen();
                printf("Saindo...\n");
                break;
            default:
                printf("Escolha invalida\n");
        }
    } while (choice != 4);

    free_memory();
    return 0;
}