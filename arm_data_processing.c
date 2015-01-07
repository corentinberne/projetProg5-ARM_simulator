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
#include "arm_data_processing.h"
#include "arm_exception.h"
#include "arm_constants.h"
#include "arm_branch_other.h"
#include "util.h"
#include "debug.h"

/* Decoding functions for different classes of instructions */
int arm_data_processing_shift(arm_core p, uint32_t ins) {
	uint8_t opcode, numRn, numRd, typeOperand, numRm, shiftDirection, shiftType;
	uint32_t rn, res, rm, shift_imm, shifter_operand;
	uint32_t cpsr = arm_read_cpsr(p);
	int n, z, v, c, shifter_carry_out, s;
	n = get_bit(cpsr, N);
	z = get_bit(cpsr, Z);
	c = get_bit(cpsr, C);
	v = get_bit(cpsr, V);
	s = get_bit(ins, 20);
	opcode = get_bits(ins, 24, 21);
	numRn = get_bits(ins, 19, 16);
	numRd = get_bits(ins, 15, 12);
	numRm = get_bits(ins, 3, 0);
	rn = arm_read_register(p, numRn);
	rm = arm_read_register(p, numRm);
	typeOperand = get_bit(ins, 4);
	if(typeOperand == 0) { //Op�rende imm�diate
		shift_imm = get_bits(ins, 11, 7);
		shiftDirection = get_bit(ins, 5);
		if(shiftDirection == LSL) {	//D�calage gauche
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else {
				shifter_operand = rm << shift_imm;
				shifter_carry_out = get_bit(rm, 32-shift_imm);
			}
		}
		else { //D�calage droit
			shiftType = get_bit(ins, 6);
			if(shiftType == 0) { //D�calage logique
				if(shift_imm == 0) {
					shifter_operand = 0;
					shifter_carry_out = get_bit(rm, 31);
				}
				else {
					shifter_operand = rm >> shift_imm;
					shifter_carry_out = get_bit(rm,shift_imm - 1);
				}
			}
			else { //D�calage arithm�tique
				if(shift_imm == 0) {
					if(get_bit(rm, 31) == 0) {
						shifter_operand = 0;
						shifter_carry_out = get_bit(rm, 31);
					}
					else {
						shifter_operand = 0xFFFFFFFF;
						shifter_carry_out = get_bit(rm, 31);
					}
				}
				else {
					shifter_operand = rm >> shift_imm;
					shifter_carry_out = get_bit(rm,shift_imm - 1);
				}
			}
		}
	}
	else { //Op�rende registre
		uint8_t numRs = get_bits(ins, 11, 8);
		shift_imm = get_bits(arm_read_register(p, numRs), 7, 0);
		shiftDirection = get_bit(ins, 5);
		if(shiftDirection == LSL) {	//D�calage gauche
			if(shift_imm == 0) {
				shifter_operand = rm;
				shifter_carry_out = c;
			}
			else if(shift_imm < 32){
				shifter_operand = rm << shift_imm;
				shifter_carry_out = get_bit(rm, 32-shift_imm);
			}
			else if(shift_imm == 32) {
				shifter_operand = 0;
				shifter_carry_out = get_bit(rm, 0);
			}
			else {
				shifter_operand = 0;
				shifter_carry_out = 0;
			}
		}
		else { //D�calage droit
			shiftType = get_bit(ins, 6);
			if(shiftType == 0) { //D�calage logique
				if(shift_imm == 0) {
					shifter_operand = rm;
					shifter_carry_out = c;
				}
				else if(shift_imm < 32){
					shifter_operand = rm >> shift_imm;
					shifter_carry_out = get_bit(rm, shift_imm-1);
				}
				else if(shift_imm == 32) {
					shifter_operand = 0;
					shifter_carry_out = get_bit(rm, 31);
				}
				else {
					shifter_operand = 0;
					shifter_carry_out = 0;
				}
			}
			else { //D�calage arithm�tique
				if(shift_imm == 0) {
					shifter_operand = rm;
					shifter_carry_out = c;
				}
				else if(shift_imm < 32){
					shifter_operand = rm >> shift_imm;
					shifter_carry_out = get_bit(rm, shift_imm-1);
				}
				else {
					if(get_bit(rm, 31) == 0) {
						shifter_operand = 0;
						shifter_carry_out = get_bit(rm, 31);
					}
					else {
						shifter_operand = 0xFFFFFFFF;
						shifter_carry_out = get_bit(rm, 31);
					}
				}
			}
		}
	}
	
	switch (opcode) {
		case 0:	//AND
			res = rn & shifter_operand;
			if(s == 1 && numRd == 15) {
				//interruption
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 1:	//EOR
			res = rn ^ shifter_operand;
			if(s == 1 && numRd == 15) {
				//interruption
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 2:	//SUB
			res = rn - shifter_operand;
			if(s == 1 && numRd == 15) {
				//interruption
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 3:	//RSB
			res = shifter_operand - rn;
			break;
		case 4:	//ADD
			res = rn + shifter_operand;
			if(s == 1 && numRd == 15) {
				//interruption
			}
			else if(s == 1) {
				n = get_bit(res, 31);
				if(res == 0)
					z = 1;
				else
					z = 0;
				c = shifter_carry_out;
			}
			break;
		case 5:	//ADC
			res = rn + shifter_operand + c;
			break;
		case 6:	//SBC
			res = rn - shifter_operand - ~c;
			break;
		case 7:	//RSC
			res = shifter_operand - rn - ~c;
			break;
		case 8:	//TST
			res = rn & shifter_operand;
			break;
		case 9:	//TEQ
			res = rn ^ shifter_operand;
			break;
		case 10:	//CMP
			res = rn - shifter_operand;
			break;
		case 11:	//CMN
			res = rn + shifter_operand;
			break;
		case 12:	//ORR
			res = rn | shifter_operand;
			break;
		case 13:	//MOV
			res = shifter_operand;
			break;
		case 14:	//BIC
			res = rn & ~shifter_operand;
			break;
		case 15:	//MVN
			res = ~shifter_operand;
			break;
	}
	if(opcode < 8 || opcode > 11)
		arm_write_register(p, numRd, res);
	if(n == 0)
		cpsr = clr_bit(cpsr, N);
	else
		cpsr = set_bit(cpsr, N);
	if(z == 0)
		cpsr = clr_bit(cpsr, Z);
	else
		cpsr = set_bit(cpsr, Z);
	if(v == 0)
		cpsr = clr_bit(cpsr, V);
	else
		cpsr = set_bit(cpsr, V);
	if(c == 0)
		cpsr = clr_bit(cpsr, C);
	else
		cpsr = set_bit(cpsr, C);
	arm_write_cpsr(p, cpsr);
	return UNDEFINED_INSTRUCTION;
}

int arm_data_processing_immediate_msr(arm_core p, uint32_t ins) {
    return UNDEFINED_INSTRUCTION;
}
