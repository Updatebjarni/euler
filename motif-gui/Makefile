CLASSES := NodeEditor Movable
CLASS_OBJS := $(addsuffix .o,$(CLASSES))

gui.o: xmtest.o class_picker.o $(CLASS_OBJS)
	ld -shared --retain-symbols-file=export.sym -o gui.o $+ \
	   `pkg-config --libs cairo` -lXm -lXt -lX11 -lXrender

$(CLASS_OBJS): %.o: %.h %P.h

%.o: %.c
	gcc -c -fPIC -o $@ -Wall `pkg-config --cflags cairo` $<

xmtest: xmtest.c NodeEditor.c NodeEditor.h NodeEditorP.h
	gcc -o xmtest xmtest.c NodeEditor.c -Wall `pkg-config --cflags --libs cairo` -lXm -lXt -lX11 -lXrender

clean:
	rm -f *.o
