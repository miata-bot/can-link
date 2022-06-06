print("hello from lua");
set_color(0xff, 0x00, 0x00);
set_brightness(255);
return;

set_brightness(0x0);

while(true)
do
  print("red");
  set_color(0xff, 0x00, 0x00);
  for i=0,255 do 
    set_brightness(i)
    -- delay_ms(1);
  end
  for i=255,0,-1 do 
    set_brightness(i)
    -- delay_ms(1);
  end
  delay_ms(500);

  print("green");
  set_color(0x00, 0xff, 0x00);
  for i=0,255 do 
    set_brightness(i)
    -- delay_ms(1);
  end
  for i=255,0,-1 do 
    set_brightness(i)
    -- delay_ms(1);
  end
  delay_ms(500);

  print("blue");
  set_color(0x00, 0x00, 0xff);
  for i=0,255 do 
    set_brightness(i)
    -- delay_ms(1);
  end
  for i=255,0,-1 do 
    set_brightness(i)
    -- delay_ms(1);
  end
  delay_ms(500);
end