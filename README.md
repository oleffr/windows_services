# windows_services
Programm for Windows Services

ЗАДАЧА: Написать программу, работающую в качестве сервиса (службы) Windows (версии 7- 10). Программа должна осуществлять резервное копирование данных из указанной директории. 

Требования: 
- Программа не должна быть интерактивной, все настройки задаются с помощью конфигурационного файла; 
- Программа должна создавать архив с резервной копией данных из указанной директории; 
- Файлы, включаемые в резервную копию, задаются по маске (с использованием символов *, ?) или непосредственно по именам; 
- Резервная копия должна создаваться в виде архива формата ZIP; 
- При изменении существующих или появлении новых файлов в исходной директории (соответствующих заданной маске) архив с резервной копией автоматически дополняется новыми данными (или заменяются существующие); 
- Инсталляция, деинсталляция, запуск, останов из командной строки (реализация своего SCP); - Корректный останов и перезапуск службы по запросу от SCM (обработка команд, передаваемых через оснастку "Службы"); 
- Имена файлов и директорий задаются в формате UNC. 
