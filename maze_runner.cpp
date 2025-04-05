#include <iostream>
#include <fstream>
#include <vector>
#include <stack>
#include <thread>
#include <chrono>
#include <cstdlib>
#include <limits>
#include <mutex>  // Inclusão do mutex


// Representação do labirinto
using Maze = std::vector<std::vector<char>>;

// Estrutura para representar uma posição no labirinto
struct Position {
    int row;
    int col;
};

// Posição inicial
Position initial_pos;

// Variáveis globais
Maze maze;
int num_rows;
int num_cols;
std::stack<Position> valid_positions;

// Mutex para proteger a impressão no console
std::mutex print_mutex; // Mutex para garantir que apenas uma thread imprima por vez

// Função para carregar o labirinto de um arquivo
Position load_maze(const std::string& file_name) {
    std::ifstream file(file_name);
    if (!file.is_open()) {
        std::cerr << "Erro ao abrir o arquivo!" << std::endl;
        exit(1);
    }

    file >> num_rows >> num_cols;
    maze.resize(num_rows);
    for (int i = 0; i < num_rows; ++i) {
        maze[i].resize(num_cols);
    }

    bool found_initial = false;
    char c;
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            file >> c;
            maze[i][j] = c;
            if (c == 'e') {
                initial_pos = {i, j};
                found_initial = true;
            }
        }
    }

    if (!found_initial) {
        std::cerr << "Erro: posição inicial ('e') não encontrada no labirinto!" << std::endl;
        exit(1);
    }

    file.close();
    return initial_pos;
}

// Função para imprimir o labirinto
void print_maze() {
    // Protege a seção crítica para que apenas uma thread imprima por vez
    std::lock_guard<std::mutex> guard(print_mutex);

    system("clear"); // No Windows, use system("cls");
    for (int i = 0; i < num_rows; ++i) {
        for (int j = 0; j < num_cols; ++j) {
            std::cout << maze[i][j];
        }
        std::cout << '\n';
    }
}

// Função para verificar se uma posição é válida
bool is_valid_position(int row, int col) {
    if (row >= 0 && row < num_rows && col >= 0 && col < num_cols) {
        if (maze[row][col] == 'x' || maze[row][col] == 's') {
            return true;
        }
    }
    return false;
}

// Função principal para navegar pelo labirinto
bool walk(Position pos) {
    // Verifique se a posição atual é a saída
    if (maze[pos.row][pos.col] == 's') {
        maze[pos.row][pos.col] = 'o'; // Marca a posição de saída como "o"
        print_maze();
        std::cout << "Labirinto Finalizado! Saída encontrada!\n";
        return true; // Encontramos a saída, retornamos imediatamente para parar a recursão
    }

    // Marque a posição atual como visitada (onde o jogador está)
    maze[pos.row][pos.col] = '.';

    // Limpar o console e imprimir o estado atual do labirinto
    print_maze();
    std::this_thread::sleep_for(std::chrono::milliseconds(50)); // Pausa para visualização

    Position aux;
    std::vector<Position> new_positions;

    // Verificar e adicionar as posições válidas adjacentes
    if (pos.col + 1 < num_cols && (maze[pos.row][pos.col + 1] == 'x' || maze[pos.row][pos.col + 1] == 's')) {
        aux.row = pos.row;
        aux.col = pos.col + 1;
        new_positions.push_back(aux);
    }
    if (pos.row + 1 < num_rows && (maze[pos.row + 1][pos.col] == 'x' || maze[pos.row + 1][pos.col] == 's')) {
        aux.row = pos.row + 1;
        aux.col = pos.col;
        new_positions.push_back(aux);
    }
    if (pos.row - 1 >= 0 && (maze[pos.row - 1][pos.col] == 'x' || maze[pos.row - 1][pos.col] == 's')) {
        aux.row = pos.row - 1;
        aux.col = pos.col;
        new_positions.push_back(aux);
    }
    if (pos.col - 1 >= 0 && (maze[pos.row][pos.col - 1] == 'x' || maze[pos.row][pos.col - 1] == 's')) {
        aux.row = pos.row;
        aux.col = pos.col - 1;
        new_positions.push_back(aux);
    }

    // Se encontrar a saída
    if (!new_positions.empty() && maze[new_positions[0].row][new_positions[0].col] == 's') {
        maze[new_positions[0].row][new_positions[0].col] = 'o';
        print_maze();
        std::cout << "\nSaída encontrada!\n";
        return true; // A saída foi encontrada
    }

    // Criar novas threads para explorar caminhos adicionais
    std::vector<std::thread> threads;
    for (size_t i = 1; i < new_positions.size(); ++i) {
        threads.emplace_back(walk, new_positions[i]);
    }

    // Explorar o primeiro caminho com a thread atual
    if (!new_positions.empty()) {
        if (walk(new_positions[0])) {
            return true; // Se encontrar a saída, retorna true
        }
    }

    // Esperar as threads adicionais terminarem
    for (auto& t : threads) {
        t.join();
    }

    return false; // Não encontrou a saída
}

int main(int argc, char* argv[]) {
    // Carregar o labirinto com o nome do arquivo recebido como argumento (argv[])
    Position initial_pos = load_maze("data/maze2.txt");

    // Iniciar a exploração a partir da posição inicial
    bool exit_found = walk(initial_pos);

    // Verificar se a saída foi encontrada
    if (exit_found) 
        std::cout << "\nSaída encontrada main!\n" << std::endl;
    else{
        std::cout << "\nNão foi possível encontrar a saída!\n" << std::endl;     
    }

    return 0;
}
