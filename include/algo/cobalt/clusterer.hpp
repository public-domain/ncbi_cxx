#ifndef ALGO_COBALT___CLUSTER_METHODS__HPP
#define ALGO_COBALT___CLUSTER_METHODS__HPP

/* $Id: clusterer.hpp 155891 2009-03-27 14:31:34Z boratyng $
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's offical duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================*/

/*****************************************************************************

File name: clusterer.hpp

Author: Greg Boratyn

Contents: Interface for CClusterer class

******************************************************************************/


#include <corelib/ncbidbg.hpp>
#include <corelib/ncbiobj.hpp>
#include <util/math/matrix.hpp>
#include <algo/phy_tree/phy_node.hpp>
#include <vector>

BEGIN_NCBI_SCOPE
BEGIN_SCOPE(cobalt)

/// Interface for CClusterer class used for clustering any type of data based
/// on distance matrix. The class operates on ideces in the distance matrix.
class NCBI_COBALT_EXPORT CClusterer
{
public:

    typedef CNcbiMatrix<double> TDistMatrix;

    /// Method for computing distance between clusters
    ///
    enum EDistMethod {
        eCompleteLinkage = 0,  ///< Maximum distance between elements
        eAverageLinkage        ///< Avegrae distance between elements
    };

    /// Single cluster
    class NCBI_COBALT_EXPORT CSingleCluster : public CObject
    {
    public:
        typedef vector<int>::const_iterator const_iterator;
        typedef vector<int>::iterator iterator;

    public:
        /// Create empty cluster
        ///
        CSingleCluster(void) : m_Prototype(-1) {}

        /// Add element to the cluster
        /// @param el Index of an element
        ///
        void AddElement(int el) {m_Elements.push_back(el);}

        /// Removes all elements from the cluster
        ///
        void clear(void) {m_Elements.clear();}

        /// Set cluster prototype
        /// @param el Index of element to be used as cluster prototype
        ///
        /// Prototype does not have to belong to the cluster
        void SetPrototype(int el) {m_Prototype = el;}

        /// Get cluster prototype
        /// @return Index of element used as cluster prototype
        ///
        int GetPrototype(void) const {return m_Prototype;}

        /// Set maximum distance in cluster
        /// @param dist Maximum distance
        ///
        void SetMaxDistance(double dist) {m_Diameter = dist;}
        
        /// Get maximum distance in cluster
        /// @return Maximum distance in cluster
        ///
        double GetMaxDistance(void) const {return m_Diameter;}

        /// Get cluster size
        /// @return Number of elements in the cluster
        ///
        size_t size(void) const {return m_Elements.size();}

        /// Find element that is closest to the center of the cluster
        /// @param dmatrix Full distance matrix used to cluster data
        /// @return Index of element that has smallest distance to all other
        /// elements
        ///
        int FindCenterElement(const TDistMatrix& dmatrix) const;

        /// Get element
        /// @param index Element index
        /// @return Element
        ///
        int operator[](size_t index) const;

        // Iterators

        const_iterator begin(void) const {return m_Elements.begin();}
        const_iterator end(void) const {return m_Elements.end();}
        iterator begin(void) {return m_Elements.begin();}
        iterator end(void) {return m_Elements.end();}

    protected:
        int m_Prototype;          ///< Index of cluster representative element
        double m_Diameter;        ///< Max distance between elements
        vector<int> m_Elements;   ///< List of indeces of cluster elements
    };

    typedef CSingleCluster TSingleCluster;
    typedef vector<TSingleCluster> TClusters;

public:

    /// Create empty clusterer
    ///
    CClusterer(void) {}

    /// Create clusterer
    /// @param dmat Distance matrix
    ///
    CClusterer(const TDistMatrix& dmat);

    /// Create clusterer
    /// @param dmat Pointer to distance matrix 
    ///
    CClusterer(auto_ptr<TDistMatrix>& dmat);

    /// Destructor
    ///
    ~CClusterer();

    /// Set new distance matrix
    /// @param dmat Distance matrix
    ///
    void SetDistMatrix(const TDistMatrix& dmat);

    /// Set new distance matrix without copying
    /// @param dmat Distance matrix
    ///
    void SetDistMatrix(auto_ptr<TDistMatrix>& dmat);

    /// Get distance matrix
    /// @return Distance matrix
    ///
    const TDistMatrix& GetDistMatrix(void) const;

    /// Compute clusters
    ///
    /// Computes complete linkage distance-based clustering with constrainted 
    /// maxium pairwise distance between cluster elements. Cluster dendrogram
    /// can be computed for each such cluster indepenently.
    /// @param max_dim Maximum distance between two elements in a cluster [in]
    /// @param dist_method Method for computing distance between clusters [in]
    /// @param do_trees If true, cluster dendrogram will be computed for each
    /// cluster [in]
    /// @param infinity Distance above which two single elements cannot be
    /// joined in a cluster. They are added to exising clusters. [in]
    ///
    void ComputeClusters(double max_diam,
                         EDistMethod dist_method = eCompleteLinkage,
                         bool do_trees = true,
                         double infinity = -1.0);

    /// Get list of elements of a specified cluster
    /// @param index Cluster index
    /// @return list of element indeces that belong to the cluster
    ///
    const TSingleCluster& GetSingleCluster(size_t index) const;

    /// Get clusters
    /// @return list of clusters
    ///
    const TClusters& GetClusters(void) const {return m_Clusters;}

    /// Set clusters
    /// @return Clusters
    ///
    TClusters& SetClusters(void) {return m_Clusters;}

    /// Get list of trees for clusters
    /// @param List of trees [out]
    ///
    void GetTrees(vector<TPhyTreeNode*>& trees) const;

    /// Get list of trees for clusters and release ownership to caller
    /// @param List of trees [out]
    ///
    void ReleaseTrees(vector<TPhyTreeNode*>& trees);

    /// Get list of trees for clusters
    /// @return List of trees
    vector<TPhyTreeNode*>& GetTrees(void) {return m_Trees;}

    /// Get tree for specific cluster
    /// @param index Cluster index [in]
    /// @return Cluster tree
    ///
    const TPhyTreeNode* GetTree(int index = 0) const;

    /// Get cluster tree and release ownership to caller
    /// @param index Cluster index [in]
    /// @return Cluster Tree
    ///
    TPhyTreeNode* ReleaseTree(int index = 0);

    /// Set prototypes for all clusters as center elements
    ///
    void SetPrototypes(void);

    /// Get distance matrix for elements of a selected cluster
    /// @param index Cluster index [in]
    /// @param mat Distance matrix for cluster elements [out]
    ///
    void GetClusterDistMatrix(int index, TDistMatrix& mat) const;

    /// Delete distance matrix
    ///
    void PurgeDistMatrix(void) {m_DistMatrix.reset();}

    /// Clear clusters and distance matrix
    ///
    void Reset(void);

protected:
    CClusterer(const CClusterer&);
    CClusterer& operator=(const CClusterer&);

protected:
    auto_ptr<TDistMatrix> m_DistMatrix;
    TClusters m_Clusters;
    vector<TPhyTreeNode*> m_Trees;
};


/// Exceptions for CClusterer class
class CClustererException : public CException
{
public:

    /// Error codes
    enum EErrCode {
        eClusterIndexOutOfRange,  ///< Index of cluster out of range
        eElemIndexOutOfRange,     ///< Index of cluster element out of range
        eElementOutOfRange,       ///< Cluster element is larger than distance
                                  ///< matrix size
        eNoDistMatrix,            ///< Distance matrix not assigned
        eInvalidOptions
    };

    NCBI_EXCEPTION_DEFAULT(CClustererException, CException);
};


END_SCOPE(cobalt)
END_NCBI_SCOPE

#endif /* ALGO_COBALT___CLUSTER_METHODS__HPP */

