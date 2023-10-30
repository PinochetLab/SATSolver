# ����������
��� ������������ ����� ����������� �� StackOverflow,
 �� ���� ��������� ������ �� �������� ���������. 
 � ����������� ����������� ��������.

 ����� �������� ����������� ������� �������� (������� ������� ����������),
 �� ����������� ������ ���� (5_6.bench) �� ~5 �����.

# ��������
���� - �������� ����� � ���, � ����� ������� UNSAT, ������ ���� ����������
�������� ������ ���������.

������� Clause simplify(Clause) ��������� �����, ����� ����������.

��������� ����� ��������� - �� ������ ���� ���������� ���.

���������� ����� ��������������:

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

��������, �������� ����������(
� ����������� � ��������� � ���������� ���������� ��� ��������, � �������, ��� ���� ������� ������� ���-�� ������������ ���. �� ���� �� �������� ��������)