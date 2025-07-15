# car_POC_6IRs_at_45degrees
<br> <br> 
Mas Rapido sigue mejor las lineas pero...
<br> 
Es lo mismo que la linea negra este entre A2 y A3 que seria el caso 0 
<br>   // Caso 0: Línea centrada (todo blanco). Asumimos que es la condición ideal.
<br>   else if (s1==1 && s2==1 && s3==1 && s4==1 && s5==1 && s6==1) {
<br>     lineaPerdidaContador = 0;
<br>     accion_AvanzarRecto();
<br>   }
<br> y Tambien es lo mismo que la linea negra este entre A4 y A5
<br> Peor aun imaginate si la linea esta a la derecha de A5 tambien vamos a estar esa misma condicion necesitamos un sensor en el centro
<br> 
<br> NECESITAMOS 7 sensores!
