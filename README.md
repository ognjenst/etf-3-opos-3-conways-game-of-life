OpenCL - Conway's game of life
---
![C++](https://img.shields.io/badge/c++-%2300599C.svg?style=for-the-badge&logo=c%2B%2B&logoColor=white)
--
This was the third assignment for the "Selected Chapters from Operating Systems" subject: Use GPU compute power to play Conway's game of life.
Find out more about the game here: [Conway's game of life](https://playgameoflife.com/)

The whole game is essentially an infinite matrix, so my approach was to assign one thread to each cell in the matrix. On the other hand, this limited the matrix size, so I went with the max values for the data type. 

For maximum memory savings, I wanted to use only one matrix in the GPU memory, but this introduces the problem of synchronization. Each cycle, every thread had to wait for all other threads to finish the last run. 
I tried to solve this, by using a memory barrier, available in OpenCL documentation, but it seems that occasionally it breaks. As a result, generated images might not be what the game run looks like. This problem remains unloved. 

## Requirements
To run this project, you will need:
* [Visual Studio](https://visualstudio.microsoft.com/)
* A dedicated GPU, AMD or NVIDIA

## Configuration 
The game has two modes. One, importing the image of correct format, with the starting position, or, second, setting the starting position in the code.
