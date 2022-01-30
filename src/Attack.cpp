#include "Board.h"

// Routines provenant de Gerbil
// Il y a plein d'informations sur le site de Bruce Moreland
//

/* Le tableau attack_array permet de savoir, pour une distance donnée entre 2 cases,
 *  si une pièce peut effectuer le déplacement.
 *
 * Par exemple, le roi blanc est en E1 (case 4)
 *              la tour noire est en E4 (case 52)
 * La distance est : 52 - 4 + 128 = 176 (on ajoute 128 pour avoir des distances positives)
 * attack_array[176] = 48
 *
 * et 48 = APF_ROOK | APF_QUEEN
 *
 * Cela signifie qu'une tour ou une dame peuvent aller de E4 à E1.
 *
 * Si la tour est en D4  -> distance = 51 - 4 + 128 = 175
 * attack_array[175] = 0 -> aucune pièce ne peut faire le déplacement
 *
 * D'autre part, il faut également connaitre comment se déplacent les pièces glissantes.
 * C'est le tableau delta_array qui le donne.
 * Dans l'exemple précédent : delta_array[176] = 16 : c'est un déplacement vertical vers le haut d'une rangée.
 *
 *
 */


static	U32 const c_argbfPc[2][7] = {
    {
        APF_NONE,
        APF_WPAWN,
        APF_KNIGHT,
        APF_BISHOP,
        APF_ROOK,
        APF_QUEEN,
        APF_KING,
    },
    {
        APF_NONE,
        APF_BPAWN,
        APF_KNIGHT,
        APF_BISHOP,
        APF_ROOK,
        APF_QUEEN,
        APF_KING,
    }
};


bool const c_argfSliding[] = {	// A "sliding" piece can traverse a ray.
    false,
    false,	// Pawn.
    false,	// Knight.
    true,	// Bishop.
    true,	// Rook.
    true,	// Queen.
    false,	// King.
};

//===================================================================
//! \brief  Initialise le tableau d'attaque avec les déplacements
//!         d'une pièce
//! \param[in]  nbr_depl    nombre maximum de déplacements (pour les pièces glissantes)
//! \param[in]  delta       déplacement
//! \param[in]  flag        flag de la pièce
//--------------------------------------------------------------------
void Board::init_ray(int nbr_depl, int delta, U32 flag)
{
    int	sq = 0;

    for (int n=0; n<nbr_depl; n++)
    {
        sq += delta;

        if ((sq + 128 >= 0) && (sq + 128 < 256))
        {
            attack_array[sq + 128] |= flag;
            delta_array[sq + 128]  = delta;
        }
    }
}

//=================================================
//! \brief Initialise les tableaux d'attaque
//!             et de déplacement
//-------------------------------------------------
void Board::init_attack()
{
    for (int i=0; i<256; i++)
    {
        attack_array[i] = 0;
        delta_array[i]  = 0;
    }

    for (auto& d : delta_wpawn)
        init_ray(1, d, APF_WPAWN);
    for (auto& d : delta_bpawn)
        init_ray(1, d, APF_BPAWN);

    for (auto& d : delta_knight)
        init_ray(1, d, APF_KNIGHT);
    for (auto& d : delta_king)
        init_ray(1, d, APF_KING);

    for (auto& e : delta_bishop)
        init_ray(7, e, APF_BISHOP);
    for (auto& e : delta_rook)
        init_ray(7, e, APF_ROOK);
    for (auto& e : delta_queen)
        init_ray(7, e, APF_QUEEN);

    for (int i=0; i<256; i++)
    {
//       printf("i=%3d a=%3d d=%3d\n", i, attack_array[i], delta_array[i]);
    }
}

//---------------------------------------------------------------------------------
//! \brief Teste si la case <s> est attaquée par le camp <c>
//! \param[in]  s   case attaquée
//! \param[in]  c   couleur du camp attaquant
//---------------------------------------------------------------------------------
bool Board::is_attacked_by(const Square s, const Color c) const
{
    int	dStep;
    int	isqAtk;
    int	pcAtk;

    const U32* flags = c_argbfPc[c];

    // Boucle sur les pièces du camp attaquant
    for (auto& p : pieces[c])
    {
        // pièce prise
        if (p.dead())
            continue;

        isqAtk = p.square();    // case où se trouve la pièce
        pcAtk  = p.type();      // type de la pièce

        // la distance entre la pièce et la case recherchée
        // correspond au flag d'attaque de cette pièce;
        if (attack_array[s - isqAtk + 128] & flags[pcAtk])
        {
            // c'est une pièce non glissante, elle attaque donc la case donnée
            if (!c_argfSliding[pcAtk])
                return true;

            // pour les pièces glissantes, il faut rechercher
            // le long de la direction de déplacement de cette pièce
            dStep = delta_array[s - isqAtk + 128];      // delta pour continuer

//            if (dStep == 0)                             // ne peut jamais arriver -> enlever le test ?
//                std::cout << "??????????????????????????????" << std::endl;

            for (;;)                                    // boucle sur la direction de déplacement
            {
                isqAtk += dStep;                        // case d'avancement
                if ((isqAtk & 0x88) != 0)            // on est en dehors de l'échiquier : fin de la recherche pour cette pièce
                    break;

                if (isqAtk == s)                        // on a atteint la case recherchée
                    return true;
                if (board[isqAtk]->type() != EMPTY)     // on a atteint une autre pièce
                    break;
            }
        }
    }

    return false;
}
