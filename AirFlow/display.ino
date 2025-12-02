int x_pos = 20;
int y_pos = 5;





void display(String texto){
  tcaselect(MultLCD);
  displayLED.clearDisplay();
  displayLED.setTextColor(WHITE);
  displayLED.setTextSize(1); 
  displayLED.setCursor(x_pos, y_pos); 
  displayLED.print(texto);
  displayLED.display();
}

void display(int numero){
  String tempString = String(numero); 
  
  tcaselect(MultLCD);
  displayLED.clearDisplay();
  displayLED.setTextColor(WHITE);
  displayLED.setTextSize(1); 
  displayLED.setCursor(x_pos, y_pos); 
  displayLED.print(tempString.c_str());
  displayLED.display();
}

void display(float numero_float){
  String tempString = String(numero_float, 2); 
  
  tcaselect(MultLCD);
  displayLED.clearDisplay();
  displayLED.setTextColor(WHITE);
  displayLED.setTextSize(1); 
  displayLED.setCursor(x_pos, y_pos); 
  displayLED.print(tempString.c_str());
  displayLED.display();
}