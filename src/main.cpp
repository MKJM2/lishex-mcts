#include <iostream>
#include "types.h"
#include "board.h"
#include "bitboard.h"
#include "uci.h"
#include "attack.h"
#include "search.h"
#include "sgd.h"

int main(int argc, char* argv[]) {

    // Print ASCII logo
    std::cout \
        << "                                                                    \n" \
        << "                      (%       ((((                                 \n" \
        << "                      (((((((  ((((((                               \n" \
        << "                      %(    ((((((#(((                              \n" \
        << "                       &((((((((((((((((                            \n" \
        << "                     ((((((((((((((((((((((                         \n" \
        << "                   (((((((((((//(((((( ((((((                       \n" \
        << "                 %((((((((/**********/(((((((                       \n" \
        << "                  ((((((//****       ****/(((((((((#                \n" \
        << "                 ((((((((*     #(((((((   *******/(                 \n" \
        << "                 ((((((((    ((((  %(((                             \n" \
        << "                 (((((((           #(((((                           \n" \
        << "                 ((((((( (         ((((((((((((                     \n" \
        << "                 ( ((((((           (((((((((((((((((               \n" \
        << "                    (((((((          (((((((((((((((((((            \n" \
        << "                     ((((((       (  %(((((((((((((((((((((         \n" \
        << "                        (((     %((  (((((((((((((((((((((((&       \n" \
        << "                          (     ((((((((((((((((((((((((((((((      \n" \
        << "                                 ((((((((((((((((((((((((((((((     \n" \
        << "              %((((((((((((((((%   (((((((((((((((((((((((((((((    \n" \
        << "           (((((((((((((((((((((((((((# ((((((((((( ((((((((((((    \n" \
        << "        ((((((((         ((((((((((((((((((((        ((((((((((((   \n" \
        << "      ((((((                 %(((((((((((((((((((((((((((((((((((   \n" \
        << "    %(((((((((                  ((((((((((((((((((((((((((((((((    \n" \
        << "   ((((    ((((((((((                (((((((( ((((((((((((((((((    \n" \
        << "                  (((((((                %((((((((((((( (((((((     \n" \
        << "                      ((((((&        (((((((((((     #(((((((       \n" \
        << "                          (((((((                 ((((((((          \n" \
        << "                               &(((((((((((((((((((((               \n" \
        << "                                                                    \n" \
        << "                  ███╗░░░███╗░█████╗░████████╗░██████╗              \n" \
        << "                  ████╗░████║██╔══██╗╚══██╔══╝██╔════╝              \n" \
        << "                  ██╔████╔██║██║░░╚═╝░░░██║░░░╚█████╗░              \n" \
        << "                  ██║╚██╔╝██║██║░░██╗░░░██║░░░░╚═══██╗              \n" \
        << "                  ██║░╚═╝░██║╚█████╔╝░░░██║░░░██████╔╝              \n" \
        << "                  ╚═╝░░░░░╚═╝░╚════╝░░░░╚═╝░░░╚═════╝░              \n" \
        << std::endl;

    // Print engine info
    std::cout << NAME << " " << VERSION << " (c) " << AUTHOR << " 2023" << std::endl;
    std::cout << "Built on " << __DATE__ << " " << __TIME__ << std::endl;

    // Initialization
    init_keys();
    init_eval_masks();
    init_leap_attacks();
    init_bishop_occupancies();
    init_rook_occupancies();
    init_magics<BISHOP>();
    init_magics<ROOK>();

    // tune();

    // Start UCI driver loop
    loop(argc, argv);

    return 0;
}
