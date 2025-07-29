#define _USE_MATH_DEFINES
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>
#include "board.h"
#include "shader.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void drawBoard();

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800; // Make it square for a square board

Board gameBoard;
StoneType currentPlayer = BLACK;
bool gameOver = false;

Shader* ourShader = nullptr;
unsigned int VBO, VAO;

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Gobang", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // build and compile our shader program
    // ------------------------------------
    ourShader = new Shader("C:/Users/Administrator/Desktop/test/src/shaders/basic.vs", "C:/Users/Administrator/Desktop/test/src/shaders/basic.fs");

    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----
        processInput(window);

        // render
        // ------
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // Draw board and stones
        drawBoard();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    // optional: de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    delete ourShader;

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// Function to draw the board and stones
void drawBoard() {
    ourShader->use();

    // Set up projection matrix (Orthographic projection for 2D)
    float projection[16];
    // glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);
    // Manual orthographic projection matrix
    projection[0] = 1.0f; projection[1] = 0.0f; projection[2] = 0.0f; projection[3] = 0.0f;
    projection[4] = 0.0f; projection[5] = 1.0f; projection[6] = 0.0f; projection[7] = 0.0f;
    projection[8] = 0.0f; projection[9] = 0.0f; projection[10] = 1.0f; projection[11] = 0.0f;
    projection[12] = 0.0f; projection[13] = 0.0f; projection[14] = 0.0f; projection[15] = 1.0f;

    ourShader->setMat4("projection", projection);

    // Draw grid lines
    std::vector<float> vertices;
    float cellSize = 2.0f / BOARD_SIZE;

    for (int i = 0; i <= BOARD_SIZE; ++i) {
        // Horizontal lines
        vertices.push_back(-1.0f); vertices.push_back(-1.0f + i * cellSize); vertices.push_back(0.0f); // Pos
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f); // Color (black)

        vertices.push_back(1.0f); vertices.push_back(-1.0f + i * cellSize); vertices.push_back(0.0f); // Pos
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f); // Color (black)

        // Vertical lines
        vertices.push_back(-1.0f + i * cellSize); vertices.push_back(-1.0f); vertices.push_back(0.0f); // Pos
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f); // Color (black)

        vertices.push_back(-1.0f + i * cellSize); vertices.push_back(1.0f); vertices.push_back(0.0f); // Pos
        vertices.push_back(0.0f); vertices.push_back(0.0f); vertices.push_back(0.0f); // Color (black)
    }

    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glBindVertexArray(VAO);
    glDrawArrays(GL_LINES, 0, vertices.size() / 6);

    // Draw stones
    for (int row = 0; row < BOARD_SIZE; ++row) {
        for (int col = 0; col < BOARD_SIZE; ++col) {
            StoneType stone = gameBoard.getStone(row, col);
            if (stone != EMPTY) {
                std::vector<float> stoneVertices;
                float x = -1.0f + (col + 0.5f) * cellSize;
                float y = -1.0f + (row + 0.5f) * cellSize;
                float radius = cellSize * 0.4f;

                // Add circle vertices
                stoneVertices.push_back(x); stoneVertices.push_back(y); stoneVertices.push_back(0.0f); // Center
                if (stone == BLACK) {
                    stoneVertices.push_back(0.0f); stoneVertices.push_back(0.0f); stoneVertices.push_back(0.0f); // Black
                } else {
                    stoneVertices.push_back(1.0f); stoneVertices.push_back(1.0f); stoneVertices.push_back(1.0f); // White
                }

                for (int i = 0; i <= 360; i += 10) {
                    float angle = i * M_PI / 180.0f;
                    stoneVertices.push_back(x + cos(angle) * radius); stoneVertices.push_back(y + sin(angle) * radius); stoneVertices.push_back(0.0f);
                    if (stone == BLACK) {
                        stoneVertices.push_back(0.0f); stoneVertices.push_back(0.0f); stoneVertices.push_back(0.0f); // Black
                    } else {
                        stoneVertices.push_back(1.0f); stoneVertices.push_back(1.0f); stoneVertices.push_back(1.0f); // White
                    }
                }
                
                glBindBuffer(GL_ARRAY_BUFFER, VBO);
                glBufferData(GL_ARRAY_BUFFER, stoneVertices.size() * sizeof(float), stoneVertices.data(), GL_STATIC_DRAW);
                glBindVertexArray(VAO);
                glDrawArrays(GL_TRIANGLE_FAN, 0, stoneVertices.size() / 6);
            }
        }
    }
}

// glfw: whenever the mouse button is pressed, this callback is called
// -------------------------------------------------------------------
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS && !gameOver) {
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);

        // Convert screen coordinates to board coordinates
        int col = static_cast<int>((xpos / SCR_WIDTH) * BOARD_SIZE);
        int row = static_cast<int>(((SCR_HEIGHT - ypos) / SCR_HEIGHT) * BOARD_SIZE); // Invert y-axis

        if (gameBoard.placeStone(row, col, currentPlayer)) {
            if (gameBoard.checkWin(row, col, currentPlayer)) {
                std::cout << (currentPlayer == BLACK ? "Black" : "White") << " wins!" << std::endl;
                gameOver = true;
            } else {
                currentPlayer = (currentPlayer == BLACK) ? WHITE : BLACK;
            }
        }
    }
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
