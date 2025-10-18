; ** por compatibilidad se omiten tildes **
; ==============================================================================
; TALLER System Programming - Arquitectura y Organizacion de Computadoras - FCEN
; ==============================================================================

%include "print.mac"
global start


; Agreguen declaraciones extern según vayan necesitando
extern print_text_rm
extern GDT_DESC
extern screen_draw_layout
extern print_hex


extern IDT_DESC
extern idt_init

extern pic_reset
extern pic_enable
extern pic_disable

extern mmu_next_free_user_page
extern mmu_next_free_kernel_page

extern mmu_init_kernel_dir
extern mmu_map_page

extern test_copy_page
extern copy_page

extern mmu_init_task_dir

extern tss_init
extern tasks_screen_draw
extern sched_init
extern tasks_init
; Definan correctamente estas constantes cuando las necesiten
%define CS_RING_0_SEL 08h
%define DS_RING_0_SEL 11000b 

%define IDLE_TASK_SEL 1100000b


BITS 16
;; Saltear seccion de datos
jmp start

;;
;; Seccion de datos.
;; -------------------------------------------------------------------------- ;;
start_rm_msg db     'Iniciando kernel en Modo Real'
start_rm_len equ    $ - start_rm_msg

start_pm_msg db     'Iniciando kernel en Modo Protegido'
start_pm_len equ    $ - start_pm_msg

;;
;; Seccion de código.
;; -------------------------------------------------------------------------- ;;

;; Punto de entrada del kernel.
BITS 16
start:
    ; Deshabilitar interrupciones
    cli 

    ; Cambiar modo de video a 80 X 50
    mov ax, 0003h
    int 10h ; set mode 03h
    xor bx, bx
    mov ax, 1112h
    int 10h ; load 8x8 font

    ; Imprimir mensaje de bienvenida - MODO REAL
    ; (revisar las funciones definidas en print.mac y los mensajes se encuentran en la
    ; sección de datos)

    ;;      %1: Puntero al mensaje
    ;;      %2: Longitud del mensaje
    ;;      %3: Color
    ;;      %4: Fila
    ;;      %5: Columna
    
    print_text_rm start_rm_msg, start_rm_len, 00001111b, 0, 0

    ; Habilitar A20
    ; (revisar las funciones definidas en a20.asm)
    call A20_enable

    ; Cargar la GDT
    LGDT [GDT_DESC]

    ; Setear el bit PE del registro CR0
    mov eax, 1
    mov cr0, eax

    ; Saltar a modo protegido (far jump)
    ; (recuerden que un far jmp se especifica como jmp CS_selector:address)
    ; Pueden usar la constante CS_RING_0_SEL definida en este archivo

    jmp CS_RING_0_SEL:modo_protegido

BITS 32
modo_protegido:
    ; A partir de aca, todo el codigo se va a ejectutar en modo protegido
    ; Establecer selectores de segmentos DS, ES, GS, FS y SS en el segmento de datos de nivel 0
    ; Pueden usar la constante DS_RING_0_SEL definida en este archivo
    mov ax, DS_RING_0_SEL
    mov ds, ax
    mov es, ax
    mov gs, ax
    mov fs, ax
    mov ss, ax
    ;Establecer el tope y la base de la pila
    mov esp, 0x25000 
    mov ebp, 0x25000
    ;agregar tope - completar - el tope es implicito 0

    ; Imprimir mensaje de bienvenida - MODO PROTEGIDO

    print_text_pm start_pm_msg, start_pm_len, 00001111b, 0, 0

    ; Inicializar pantalla
    
    call screen_draw_layout
   
    ; Inicializar el directorio de paginas
    call mmu_init_kernel_dir

    ; Cargar directorio de paginas
    and eax, 0xFFFFF000
    mov cr3, eax

    ; Habilitar paginacion
    mov eax, cr0 
    or eax, 1000_0000_0000_0000_0000_0000_0000_0000b
    mov cr0, eax

    ; Inicializar tss
    call tss_init

    ; Inicializar el scheduler
    call sched_init

    ; Inicializar las tareas
    call tasks_init

    ; Inicializar y cargar la IDT
    call idt_init
    LIDT [IDT_DESC]

    ; Reiniciar y habilitar el controlador de interrupciones
    call pic_reset
    call pic_enable

    ; Cargar tarea inicial
    call tasks_screen_draw

    mov eax, 11
    shl eax, 3
    LTR ax
    
    ; Habilitar interrupciones
    sti
    ; NOTA: Pueden chequear que las interrupciones funcionen forzando a que se
    ;       dispare alguna excepción (lo más sencillo es usar la instrucción
    ;       `int3`)
    ;int3

    ; Probar Sys_cal
    int 88
    ; Probar generar una excepción

    ; Inicializar el directorio de paginas de la tarea de prueba
    push 0x18000
    call mmu_init_task_dir
    add esp, 4

    ; Cargar directorio de paginas de la tarea
    mov esi, cr3
    mov cr3, eax    

    ;mov BYTE [0x7000000], 1    ;escriura sobre pagina on demand
    ;mov BYTE [0x7000008], 2

    ; Restaurar directorio de paginas del kernel
    mov cr3, esi

    ;call test_copy_page

    ;modificando PIT

    mov ax, 4444
    out 0x40, al
    rol ax, 8
    out 0x40, al

    ; Saltar a la primera tarea: Idle
    jmp IDLE_TASK_SEL:0

   

    ; Ciclar infinitamente 
    mov eax, 0xFFFF
    mov ebx, 0xFFFF
    mov ecx, 0xFFFF
    mov edx, 0xFFFF
    jmp $

;; -------------------------------------------------------------------------- ;;

%include "a20.asm"
