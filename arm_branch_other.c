/*
Armator - simulateur de jeu d'instruction ARMv5T � but p�dagogique
Copyright (C) 2011 Guillaume Huard
Ce programme est libre, vous pouvez le redistribuer et/ou le modifier selon les
termes de la Licence Publique G�n�rale GNU publi�e par la Free Software
Foundation (version 2 ou bien toute autre version ult�rieure choisie par vous).

Ce programme est distribu� car potentiellement utile, mais SANS AUCUNE
GARANTIE, ni explicite ni implicite, y compris les garanties de
commercialisation ou d'adaptation dans un but sp�cifique. Reportez-vous � la
Licence Publique G�n�rale GNU pour plus de d�tails.

Vous devez avoir re�u une copie de la Licence Publique G�n�rale GNU en m�me
temps que ce programme ; si ce n'est pas le cas, �crivez � la Free Software
Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307,
�tats-Unis.

Contact: Guillaume.Huard@imag.fr
         ENSIMAG - Laboratoire LIG
         51 avenue Jean Kuntzmann
         38330 Montbonnot Saint-Martin
*/
#include "arm_branch_other.h"
#include "arm_constants.h"
#include "util.h"
#include <debug.h>
#include <stdlib.h>


int arm_branch(arm_core p, uint32_t ins) {
	
	// B / BL
		// R�cup�ration du PC actuel
	int PC = arm_read_register(p, 15);
		// Enregistrement du PC si le brachement est link�
	if (get_bit(ins, 24) == 1) {
		arm_write_register(p, 14, PC-4);
	}
		// Traitement de l'offset (de 24 � 32 bit)
	int offset = asr(ins << 8, 8) * 4;
		// Modification du PC
	int newPC = PC + offset;
	arm_write_register(p, 15, (uint32_t) newPC);

	return 0;
}

int arm_coprocessor_others_swi(arm_core p, uint32_t ins) {

	// SWI
	if (get_bit(ins, 24)) {
		if ((ins & 0xFFFFFF) == 0x123456)
			exit(0);
		return SOFTWARE_INTERRUPT;
	} 
	
	return UNDEFINED_INSTRUCTION;
}

int arm_miscellaneous(arm_core p, uint32_t ins) {

	// MRS
	printf("%d, %d", get_bits(ins,27,23), get_bits(ins,21,20));
	if (get_bits(ins,27,23) == 2 && get_bits(ins,21,20) == 0) { 
		if (get_bit(ins,22) == 0) // CPSR -> Rd
			arm_write_register(p,get_bits(ins,15,12),arm_read_cpsr(p));
		else			  // SPSR -> Rd
			arm_write_register(p,get_bits(ins,15,12),arm_read_spsr(p));
		return 0;
	}

	return UNDEFINED_INSTRUCTION;
}
