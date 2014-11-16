target remote localhost:3333
monitor reset halt


define frun
  monitor reset halt
  load $arg0
  monitor reset run
end

define halt
 monitor reset halt
end

define reset
 monitor reset halt
 monitor reset run
end
