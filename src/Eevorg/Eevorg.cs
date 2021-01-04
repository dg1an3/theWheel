using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;

namespace Eevorg
{
    public class Eevorg
    {
        /// <summary>
        /// 
        /// </summary>
        public Eevorg()
        {
            // the generator for the recursive ca1d
            Func<Func<int, int, int>, Func<int, int, int>> genCa1d = 
                ca1d => (x, t) => 
                    (x > t) ? 0 
                        : (t == 0) ? 1 
                            : rule[ca1d(x-1, t-1) + ca1d(x, t-1) + ca1d(x+1, t-1)];

            // produce the evaluator function
            eval = YCombinator(genCa1d);
        }

        /// <summary>
        /// the rule to use to generate subsequent lines of the Ca1D Eevorg
        /// </summary>
        public int[] rule;

        // the evaluation function
        public Func<int, int, int> eval;

        // stores the cells
        public int[,] cells;

        /// <summary>
        /// 
        /// </summary>
        /// <param name="nGeneration"></param>
        public int[,] EvaluateCells(int nGenerations)
        {
            cells = new int[nGenerations, nGenerations];
            for (int nGenAt = 0; nGenAt < nGenerations; nGenAt++)
            {
                for (int nCellAt = 0; nCellAt <= nGenAt; nCellAt++)
                {
                    cells[nGenAt, nCellAt] = eval(nGenAt, nCellAt);
                }
            }

            return cells;
        }

        /// <summary>
        /// 
        /// </summary>
        /// <typeparam name="T"></typeparam>
        /// <param name="self"></param>
        /// <returns></returns>
        delegate T SelfApplicable<T>(SelfApplicable<T> self);

        /// <summary>
        /// 
        /// </summary>
        /// <param name="genFunc"></param>
        /// <returns></returns>
        public Func<int, int, int> YCombinator(Func<Func<int, int, int>, Func<int, int, int>> genFunc)
        {
            // form the Y-combinator
            SelfApplicable< Func<Func<Func<int, int, int>, Func<int, int, int>>, Func<int, int, int>>> Y =
                y => f => (x, t) => f(y(y)(f))(x, t);

            // apply the fixed-point function to generate the final recursive function
            return Y(Y)(genFunc);
        }
    }
}
