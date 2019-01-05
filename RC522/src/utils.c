/* utilities and other stuff
 * Copyright © 2018 tyrolyean
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <stddef.h>

int replace_char(char* str, char old, char rpl){
        
        for(int i = 0; str[i] != 0; i++){
                if(str[i] == old){
                        str[i] = rpl;
                }
        }

        return 0;
}

size_t charcnt(char* str, char find){
        size_t cnt = 0;
        for(int i = 0; str[i] != 0; i++){
                if(str[i] == find){
                    cnt++;    
                }
        }
        return cnt;
}
