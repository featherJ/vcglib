/****************************************************************************
* VCGLib                                                            o o     *
* Visual and Computer Graphics Library                            o     o   *
*                                                                _   O  _   *
* Copyright(C) 2004                                                \/)\/    *
* Visual Computing Lab                                            /\/|      *
* ISTI - Italian National Research Council                           |      *
*                                                                    \      *
* All rights reserved.                                                      *
*                                                                           *
* This program is free software; you can redistribute it and/or modify      *   
* it under the terms of the GNU General Public License as published by      *
* the Free Software Foundation; either version 2 of the License, or         *
* (at your option) any later version.                                       *
*                                                                           *
* This program is distributed in the hope that it will be useful,           *
* but WITHOUT ANY WARRANTY; without even the implied warranty of            *
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             *
* GNU General Public License (http://www.gnu.org/licenses/gpl.txt)          *
* for more details.                                                         *
*                                                                           *
****************************************************************************/
/****************************************************************************
  History

$Log: not supported by cvs2svn $

****************************************************************************/

#ifndef __VCG_GEODESIC
#define __VCG_GEODESIC

namespace vcg {
	
template <class MESH>
class VQualityHeap
{
public:
	float q;
	MESH::vertex_pointer p;
	inline VQualityHeap( MESH::vertex_pointer np )
	{
		q = np->Q();
		p = np;
	}
		// Attenzione il minore e' maggiore
	inline bool operator <  ( const VQualityHeap & vq ) const { return q >  vq.q; }
	inline bool operator == ( const VQualityHeap & vq ) const { return q == vq.q; }
	inline bool operator >  ( const VQualityHeap & vq ) const { return q <  vq.q; }
	inline bool operator != ( const VQualityHeap & vq ) const { return q != vq.q; }
	inline bool operator <= ( const VQualityHeap & vq ) const { return q >= vq.q; }
	inline bool operator >= ( const VQualityHeap & vq ) const { return q <= vq.q; }
	inline bool is_valid() const { return q==p->Q(); }
};

// Calcola la qualita' come distanza geodesica dal bordo della mesh.
// Robusta funziona anche per mesh non manifold.
// La qualita' memorizzata indica la distanza assoluta dal bordo della mesh.
// Nota prima del 13/11/03 in alcuni casi rari SPT andava in loop perche' poteva capitare
// che per approx numeriche ben strane pw->Q() > pv->Q()+d ma durante la memorizzazione 
// della nuova distanza essa rimanesse uguale a prima. Patchato rimettendo i vertici nello 
// heap solo se migliorano la distanza di un epsilon == 1/100000 della mesh diag.

template <class MESH>
void ComputeGeodesicQuality(MESH &m, bool per_face )	// R1
{
	//Requirements
	assert(m.HasVFTopology());
	assert(m.HasPerVertexQuality());
  if(per_face) assert(m.HasPerFaceQuality());

	vector<VQualityHeap<MESH> > heap;
	MESH::vertex_iterator v;
	MESH::face_iterator   f;
	int j;

	m.VFTopology();									// Creazione adiacenza vertici
	m.ComputeBorderFlagVF();				// Marco gli edge di bordo

	for(v=m.vert.begin();v!=m.vert.end();++v)
		(*v).Q() = -1;
	for(f=m.face.begin();f!=m.face.end();++f)			// Inserisco nell'heap i v di bordo
		if(!(*f).IsD())
			for(j=0;j<3;++j)
				if( (*f).IsB(j) )
				{
					for(int k=0;k<2;++k)
					{
						MESH::vertex_pointer pv = (*f).V((j+k)%3);
						if( pv->Q()==-1 )
						{
							pv->Q() = 0;
							heap.push_back(VQualityHeap<MESH>(pv));
						}
					}
				}
	
 const MESH::scalar_type loc_eps=m.bbox.Diag()/MESH::scalar_type(100000);
 while( heap.size()!=0 )							// Shortest path tree
	{
		MESH::vertex_pointer pv;
		pop_heap(heap.begin(),heap.end());
		if( ! heap.back().is_valid() )
		{
			heap.pop_back();
			continue;
		}
		pv = heap.back().p;
		heap.pop_back();
	 MESH::vedgepos_type x;
		for( x.f = pv->Fp(), x.z = pv->Zp(); x.f!=0; x.NextF() )
		{
			for(int k=0;k<2;++k)
			{
				MESH::vertex_pointer pw;
				float d;
				if(k==0) pw = x.f->V1(x.z);
				else     pw = x.f->V2(x.z);
				d = Distance(pv->P(),pw->P());
				if( pw->Q()==-1 || pw->Q() > pv->Q()+d + loc_eps)
				{
					pw->Q() = pv->Q()+d;
					heap.push_back(VQualityHeap<MESH>(pw));
					push_heap(heap.begin(),heap.end());
				}
			}
		}
	}

	for(v=m.vert.begin();v!=m.vert.end();++v)
		if(v->Q()==-1)
			v->Q() = 0;

	if(per_face)
	{
		for(f=m.face.begin();f!=m.face.end();++f)
			(*f).Q() = ((*f).V(0)->Q() + (*f).V(1)->Q() + (*f).V(2)->Q())/3;
	}
}





#endif