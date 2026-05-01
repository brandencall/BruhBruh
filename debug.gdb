# save as debug.gdb and run with:
# gdb -x debug.gdb ./build/BruhBruh

set pagination off
set print thread-events on

# Break on malloc/free to catch the specific address
break malloc
commands
  silent
  if $rax == 0x7c1ff60b12f0
    printf "ALLOCATED at 0x7c1ff60b12f0\n"
    bt
  end
  continue
end

break free
commands
  silent
  if $rdi == 0x7c1ff60b12f0
    printf "FREED at 0x7c1ff60b12f0\n"
    bt
    info threads
  end
  continue
end

run
