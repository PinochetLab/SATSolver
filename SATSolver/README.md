# Результаты
Все предложенные тесты завершаются со StackOverflow,
 на моих небольших тестах всё работает корректно. 
 Я использовал рекурсивный алгоритм.

 Также пробовал реализовать наивный алгоритм (пытаясь угадать переменную),
 он обрабатывал первый тест (5_6.bench) за ~5 минут.

# Алгоритм
Идея - привести схему к ДНФ, а потом вернуть UNSAT, только если получилось
заведомо ложное выражение.

Функция Clause simplify(Clause) принимает схему, выдаёт упрощённую.

Инвариант моего алгоритма - на каждом шаге получается ДНФ.

Получаются такие преобразования:

- simplify !!A -> simplify A
- simplify !(A & B) -> simplify !A | simplify !B
- simplify !(A | B) -> simplify !A & simplify !B
- simplify (A & A) -> simplify A
- simplify (A | A) -> simplify A
- simplify (A & !A) -> False
- simplify (A | !A) -> True
- simplify (A & True) -> simplify A
- simplify (A & False) -> False
- simplify (A | True) -> True
- simplify (A | False) -> simplify A
- simplify (A & (B & C)) -> simplify A & simplify B & simplify C
- simplify (A | (B | C)) -> simplify A | simplify B | simplify C
- simplify ((A | B) & (C | D)) -> simplify (A & C) | simplify (A & D) | simplify (B & C) | simplify (B & D)

Очевидно, алгоритм слабоватый(
Я ознакомился в интернете с различными вариациями Сат Солверов, и подумал, что цель задания сделать что-то относительно своё. Ну хотя бы работает корректо)