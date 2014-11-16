openocd&
arm-none-eabi-gdb name.elf

monitor reset halt
load name.elf
monitor reset run

# mittels b <Funktionsname> breakpoint setzen
# s -> step weise
# list -> aktuelle Position ausgeben


Alternativ
openocd
arm-none-eabi-gdb -x command.gdb

frun name.elf  -> Zurücksetzen, Flashen, Ausführen
reset           -> Zurücksetzen und Ausführen
halt           -> Zurücksetzen und anhalten 
