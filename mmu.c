/* ** por compatibilidad se omiten tildes **
================================================================================
 TRABAJO PRACTICO 3 - System Programming - ORGANIZACION DE COMPUTADOR II - FCEN
================================================================================

  Definicion de funciones del manejador de memoria
*/

#include "mmu.h"
#include "i386.h"

#include "kassert.h"
#include "defines.h"

static pd_entry_t* kpd = (pd_entry_t*)KERNEL_PAGE_DIR;
static pt_entry_t* kpt = (pt_entry_t*)KERNEL_PAGE_TABLE_0;

static const uint32_t identity_mapping_end = 0x003FFFFF;
static const uint32_t user_memory_pool_end = 0x02FFFFFF;

static paddr_t next_free_kernel_page = 0x100000;
static paddr_t next_free_user_page = 0x400000;

/**
 * kmemset asigna el valor c a un rango de memoria interpretado
 * como un rango de bytes de largo n que comienza en s
 * @param s es el puntero al comienzo del rango de memoria
 * @param c es el valor a asignar en cada byte de s[0..n-1]
 * @param n es el tamaño en bytes a asignar
 * @return devuelve el puntero al rango modificado (alias de s)
*/
static inline void* kmemset(void* s, int c, size_t n) {
  uint8_t* dst = (uint8_t*)s;
  for (size_t i = 0; i < n; i++) {
    dst[i] = c;
  }
  return dst;
}

/**
 * zero_page limpia el contenido de una página que comienza en addr
 * @param addr es la dirección del comienzo de la página a limpiar
*/
static inline void zero_page(paddr_t addr) {	// si, solo opera sobre identity mapping
  kmemset((void*)addr, 0x00, PAGE_SIZE);
}


void mmu_init(void) {}


/**
 * mmu_next_free_kernel_page devuelve la dirección física de la próxima página de kernel disponible. 
 * Las páginas se obtienen en forma incremental, siendo la primera: next_free_kernel_page
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de kernel
 */
paddr_t mmu_next_free_kernel_page(void) {
  next_free_kernel_page = next_free_kernel_page + 4096;
  return next_free_kernel_page - 4096;
}

/**
 * mmu_next_free_user_page devuelve la dirección de la próxima página de usuarix disponible
 * @return devuelve la dirección de memoria de comienzo de la próxima página libre de usuarix
 */
paddr_t mmu_next_free_user_page(void) {
  next_free_user_page = next_free_user_page + 4096;
  return next_free_user_page - 4096;
}

/**
 * mmu_init_kernel_dir inicializa las estructuras de paginación vinculadas al kernel y
 * realiza el identity mapping
 * @return devuelve la dirección de memoria de la página donde se encuentra el directorio
 * de páginas usado por el kernel
 */
paddr_t mmu_init_kernel_dir(void) {
  const uint32_t page_directory_dir = 0x25000;

  //objetivo: iniciar un page directory para el kernel, y mappear las paginas 
  zero_page(page_directory_dir);
  paddr_t page_table_location = mmu_next_free_kernel_page();

  pd_entry_t* directory_entry = (pd_entry_t*) page_directory_dir; // solo necesitamos una tabla
  
  directory_entry->pt = page_table_location >> 12;
  //4 reservados, bit 7 0, ignored 0, accessed 0, pcd 0, pwt 0, u/s 0, r/w 1, P 1
  uint16_t attributes = 0x3;//0b000000000011
  directory_entry->attrs = attributes;

  zero_page(page_table_location);
     //1023  = identity_mapping_end / 4096
  for(int i = 0; i < 1024 ; i++){

    pt_entry_t* table_entry = (pt_entry_t*) (page_table_location + i*4); 
  
    table_entry->page = i;

    uint16_t attributes = 0x107;//0b0001 0000 0111;
                    //         |-global (no se invalida cuando cambia cr3)

    table_entry->attrs = attributes;
  }
  return page_directory_dir;

}

/**
 * mmu_map_page agrega las entradas necesarias a las estructuras de paginación de modo de que
 * la dirección virtual virt se traduzca en la dirección física phy con los atributos definidos en attrs
 * @param cr3 el contenido que se ha de cargar en un registro CR3 al realizar la traducción
 * @param virt la dirección virtual que se ha de traducir en phy
 * @param phy la dirección física que debe ser accedida (dirección de destino)
 * @param attrs los atributos a asignar en la entrada de la tabla de páginas
 */
void mmu_map_page(uint32_t cr3, vaddr_t virt, paddr_t phy, uint32_t attrs) {
    pd_entry_t* page_directory = (pd_entry_t*) CR3_TO_PAGE_DIR(cr3);

    uint32_t pd_index = VIRT_PAGE_DIR(virt);
                                                                    
    if(page_directory[pd_index].attrs == 0){                   
        paddr_t page_table_location = mmu_next_free_kernel_page();

        pd_entry_t* directory_entry = ((pd_entry_t*) page_directory) + pd_index; 
        directory_entry->pt = page_table_location >> 12;
    
        directory_entry->attrs = (attrs | 0x2);

        zero_page(page_table_location);
    }

    pt_entry_t* page_table_address = (pt_entry_t*) (page_directory[pd_index].pt << 12);
    
    uint32_t pt_index = VIRT_PAGE_TABLE(virt);

    if(page_table_address[pt_index].attrs == 0){      
        pt_entry_t* table_entry = ((pt_entry_t*) (page_table_address)) + pt_index; 
    
        table_entry->page = phy >> 12; // asumimos que la direccion fisica es de 32 bits
        table_entry->attrs = attrs;

    }
    
}

/**
 * mmu_unmap_page elimina la entrada vinculada a la dirección virt en la tabla de páginas correspondiente
 * @param virt la dirección virtual que se ha de desvincular
 * @return la dirección física de la página desvinculada
 */
paddr_t mmu_unmap_page(uint32_t cr3, vaddr_t virt) {
    pd_entry_t* page_directory = (pd_entry_t*)CR3_TO_PAGE_DIR(cr3);
    uint32_t pd_index = VIRT_PAGE_DIR(virt);
    
    paddr_t page = 0;
    if (page_directory[pd_index].attrs != 0){
        pt_entry_t* page_table_directory = (pt_entry_t*) (page_directory[pd_index].pt << 12);
        uint32_t pt_index = VIRT_PAGE_TABLE(virt);

        page = page_table_directory[pt_index].page;
        page_table_directory[pt_index].attrs = 0;
        page_table_directory[pt_index].page = 0;
    }
    return page;
}

#define DST_VIRT_PAGE 0xA00000
#define SRC_VIRT_PAGE 0xB00000

/**
 * copy_page copia el contenido de la página física localizada en la dirección src_addr a la página física ubicada en dst_addr
 * @param dst_addr la dirección a cuya página queremos copiar el contenido
 * @param src_addr la dirección de la página cuyo contenido queremos copiar
 *
 * Esta función mapea ambas páginas a las direcciones SRC_VIRT_PAGE y DST_VIRT_PAGE, respectivamente, realiza
 * la copia y luego desmapea las páginas. Usar la función rcr3 definida en i386.h para obtener el cr3 actual
 */ 
void copy_page(paddr_t dst_addr, paddr_t src_addr) {
    uint32_t current_cr3 = rcr3();
    mmu_map_page(current_cr3, SRC_VIRT_PAGE, src_addr, 0x3);
    mmu_map_page(current_cr3, DST_VIRT_PAGE, dst_addr, 0x3);

	uint8_t* src = (uint8_t*)(SRC_VIRT_PAGE);
  	uint8_t* dst = (uint8_t*)(DST_VIRT_PAGE);

    for(int i = 0; i < PAGE_SIZE; i++){
        dst[i] = src[i];
    }

    mmu_unmap_page(current_cr3, SRC_VIRT_PAGE);
    mmu_unmap_page(current_cr3, DST_VIRT_PAGE);

}

// Esta es una funcion que utilizamos para berificar el correcto funcionamiento de copy_page()
void test_copy_page(){
	paddr_t dir1 = mmu_next_free_kernel_page();
	paddr_t dir2 = mmu_next_free_kernel_page();

	vaddr_t dir3 = 0x401000;
  uint32_t current_cr3 = rcr3();

	mmu_map_page(current_cr3, dir3, dir1, 0x3);
	
	uint32_t* inicio = (uint32_t*) dir3;
	*inicio = 1;

	mmu_unmap_page(current_cr3, dir3);

	copy_page(dir2, dir1);
}

 /**
 * mmu_init_task_dir inicializa las estructuras de paginación vinculadas a una tarea cuyo código se encuentra en la dirección phy_start
 * @pararm phy_start es la dirección donde comienzan las dos páginas de código de la tarea asociada a esta llamada
 * @return el contenido que se ha de cargar en un registro CR3 para la tarea asociada a esta llamada
 */
paddr_t mmu_init_task_dir(paddr_t phy_start) {
    uint32_t current_cr3 = rcr3();

	// init PD
	paddr_t page_for_page_directory = mmu_next_free_kernel_page();
	zero_page(page_for_page_directory);

  	pd_entry_t kernel_pt_address = *(pd_entry_t*) CR3_TO_PAGE_DIR(current_cr3);
	kernel_pt_address.attrs = 0x5;//0b000000000101 

	pd_entry_t* task_page_directory = (pd_entry_t*)page_for_page_directory;
	*task_page_directory = kernel_pt_address; 

	// init code pages
	for(int i = 0; i < TASK_CODE_PAGES; i++){
		uint32_t attrs = 5; // read only, present, user

		mmu_map_page(page_for_page_directory, TASK_CODE_VIRTUAL + i*PAGE_SIZE, phy_start + i*PAGE_SIZE, attrs);
	}

  // init stack page
	paddr_t page_for_stack = mmu_next_free_user_page();
	uint32_t stack_attrs = 7;
	mmu_map_page(page_for_page_directory, TASK_STACK_BASE - 0x1000, page_for_stack, stack_attrs);

  // init shared
	uint32_t shared_attrs = 7;	
	mmu_map_page(page_for_page_directory, TASK_SHARED_PAGE, SHARED, shared_attrs);

	return page_for_page_directory;
}

// Devuelve true si se atendió el page fault y puede continuar la ejecución 
// y false si no se pudo atender

bool page_fault_handler(vaddr_t virt) {
  print("Atendiendo page fault...", 0, 0, C_FG_WHITE | C_BG_BLACK);
  // Chequeemos si el acceso fue dentro del area on-demand
  // En caso de que si, mapear la pagina (que es solo 1)
  uint32_t current_cr3 = rcr3();
  if(virt >= ON_DEMAND_MEM_START_VIRTUAL && virt <= ON_DEMAND_MEM_END_VIRTUAL){
      mmu_map_page(current_cr3, ON_DEMAND_MEM_START_VIRTUAL, ON_DEMAND_MEM_START_PHYSICAL, 7);
      return true;
  }
  return false;
}


