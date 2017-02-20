# Controller of water heater
Sterownik podgrzewacza wody (Atmega8 + Eclipse + C++)

W starym, 20-letnim podgrzewaczu popsuła się elektronika. Ponieważ sam podgrzewacz był sprawny, przy uzyciu kilku części zbudowałem układ oparty o mikroprocesor Atmega8. Układ ten ma za zadanie generować (przy uzyciu przerwań) impuls, który zostanie przekazany na optotriaka. Zależnie od częstości wystąpienia impulsu podgrzewacz będzie działał z różna mocą. jest to wazne, ponieważ bezpiecznik jest słabszy niż maksymalna moc podgrzewacza.

Układ umozliwia ustawienie temperatury, do której ma być podgrzana woda. Gdy temperatura jest niższa niż ustawiona, układ rozpoczyna generowanie impulsów. Gdy temperatura zostanie osiągnięta, jest to przerywane.

Jest mozliwość zapauzowania pracy układu bez względu na obecny stan.

Ustawienia są automatycznie zapisywane przy przygaśnięciu ekranu (o ile zostały dokonane).

Obecny stan ustawień i odczyt temperatury jest na bieżąco wyswietlany na ekranie.

Wykorzystano gotowe biblioteki obsługi wyswietlacza i termometru wzbogacając je o dodatkowe mozliwości wynikajace z programowania w C++ (biblioteki były pisane pod C).
