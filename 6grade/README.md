# Схема решения на 6 баллов.
Программа использует три процесса и два неименованных канала для конвейерной обработки текстового файла.

Более подробно, решение задачи включает в себя следующие шаги:

1. Открытие и чтение входного файла в первом процессе.
2. Создание первого неименованного канала для связи между первым и вторым процессами.
3. Создание второго неименованного канала для связи между вторым и третьим процессами.
4. Создание двух дочерних процессов, которые будут обрабатывать данные из первого и второго каналов соответственно.
5. Запись данных в первый канал из первого процесса.
6. Чтение данных из первого канала вторым процессом, обработка данных и запись их во второй канал.
7. Чтение данных из второго канала первым процессом и запись их в выходной файл.
8. Ожидание завершения работы дочерних процессов и закрытие всех каналов и файловых дескрипторов.

## Результаты работы программы
### Тест 1
```
Input: Hello World!
Output: HeLLo WoRLD!
```

### Тест 2
```
Input: The quick brown fox jumps over the lazy dog.
Output: THe QuiCK BRoWN FoX JuMPS oVeR THe LaZy DoG.
```

### Тест 3
```
Input: AEIOUaeiou
Output: AEIOUaeiou
```

### Тест 4
```
Input: 12345
Output: 12345
```

### Тест 5
```
Input: 
Output: 
```