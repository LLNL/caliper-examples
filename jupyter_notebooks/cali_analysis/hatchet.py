from . import util

import json
import pandas as pd


class TreeMetaColumns(object):
    def __init__(self, existing_columns, hierarchical_column, inclusive_aggregators):
        # TODO: Ensure that generated columns do not collide with existing columns
        self.hierarchical_column = hierarchical_column

        self.hierarchy_depth = 'depth#' + hierarchical_column
        self.hash_column = 'hash#' + hierarchical_column
        self.parent_hash_column = 'parent_hash#' + hierarchical_column

        self.inclusive_aggregators = inclusive_aggregators.copy()
        self.inclusive_aggregators.update({
            self.hierarchical_column: 'first',
            self.parent_hash_column: 'first',
            self.hierarchy_depth: 'first'
        })

    def copy_tree_meta_columns(self, other):
        self.hierarchical_column = other.hierarchical_column
        self.hierarchy_depth = other.hierarchy_depth
        self.hash_column = other.hash_column
        self.parent_hash_column = other.parent_hash_column
        self.inclusive_aggregators = other.inclusive_aggregators


class ObservedDataFrame(pd.DataFrame):
    """ Subclass of DataFrame that observes modifications and executes a callback after each one """

    def __init__(self, df, post_modification_cb=lambda *args: None):
        super(ObservedDataFrame, self).__init__(df)
        self.post_modification_cb = post_modification_cb

    def __setitem__(self, key, value):
        pd.DataFrame.__setitem__(self, key, value)
        self.post_modification_cb()

    def _update_inplace(self, result, verify_is_copy=True):
        pd.DataFrame._update_inplace(self, result, verify_is_copy)
        self.post_modification_cb()


class MultiRootTree(TreeMetaColumns):

    def __init__(self, df_samples, tree_meta_columns):
        self.copy_tree_meta_columns(tree_meta_columns)
        self._df_samples = ObservedDataFrame(df_samples, self.regenerate_nodes)
        self._df_nodes = self.generate_nodes()

    def __repr__(self):
        """ Generate a JSON tree for all roots and combine into a list """

        return util.pretty_json_dumps(self.to_json())

    def to_json(self):
        return [self.to_json_tree(root) for root in self.roots()]

    def regenerate_nodes(self):
        self._df_nodes = self.generate_nodes()

    @property
    def df_samples(self):
        return self._df_samples

    @df_samples.setter
    def df_samples(self, value):
        """ When modifying the samples DataFrame, regenerate the forest """

        self._df_samples = value
        self.regenerate_nodes()

    @property
    def df_nodes(self):
        return self._df_nodes

    @df_nodes.setter
    def df_nodes(self, value):
        """ Modifying the forest nodes directly may invalidate the samples """

        raise Exception("You aren't allowed to do that!")

    def query_roots(self):
        """ Query all nodes with depth of 1, meaning roots (as a DataFrame) """

        return self._df_nodes[self._df_nodes[self.hierarchy_depth] == 1]

    def query_children_of(self, node):
        """ Given a node, query its children (as a DataFrame) """

        node_hash = node[self.hash_column]
        return self._df_nodes[self._df_nodes[self.parent_hash_column] == node_hash]

    def query_path(self, node, child_selector):
        """ Return a DataFrame with a path through the tree rooted at `node`,
         where the path is determined at each level by `child_selector` """

        children = self.children_of(node)
        if children:
            return pd.DataFrame([node]).append(self.query_path(child_selector(children), child_selector))
        return pd.DataFrame([node])

    @property
    def roots(self):
        """ Convenience function to get roots as a list of Node instances """

        roots = self.query_roots()
        return [Node(roots.iloc[i], self) for i in range(len(roots))]

    def children_of(self, node):
        """ Convenience function to get children of a node as a list of Node instances """

        children = self.query_children_of(node)
        return [Node(children.iloc[i], self) for i in range(len(children))]

    def to_json_tree(self, node):
        """ Generate a tree, rooted at the given node, as a JSON object """

        node_dict = json.loads(node.to_json())
        children = self.query_children_of(node)
        if not children.empty:
            node_dict['children'] = [self.to_json_tree(children.iloc[i]) for i in range(len(children))]
        return node_dict

    def node_generator(self):
        """ Iteratively generates inclusive nodes bottom-up from
            a dataframe of samples and a dict of per-column aggregators """

        # Get the longest path length
        max_path = max([len(p) for p in self._df_samples[self.hierarchical_column]])

        # Work on an internal copy of the samples dataframe
        df_nodes = self._df_samples.copy()

        for i in range(max_path, 0, -1):
            # Get the subpath, iterating from longest to shortest
            df_nodes[self.hierarchical_column] = df_nodes[self.hierarchical_column].str[:i]

            # Calculate depth in hierarchy
            df_nodes[self.hierarchy_depth] = df_nodes[self.hierarchical_column].transform(len)

            # Hash the subpath at this sample
            df_nodes[self.hash_column] = df_nodes[self.hierarchical_column].transform(
                lambda l: hash(l))

            # Hash the parent subpath
            df_nodes[self.parent_hash_column] = df_nodes[self.hierarchical_column].transform(
                lambda l: hash(l[:-1]))

            # Aggregate all entries with a common subpath using the provided aggregators
            df_nodes = df_nodes.groupby(self.hash_column, as_index=False).agg(
                self.inclusive_aggregators)

            # Produce a set of nodes at this hierarchy depth
            yield df_nodes[df_nodes[self.hierarchy_depth] == i].copy()

    def generate_nodes(self):
        """ Generate all inclusive nodes and concatenate into a single DataFrame representing the forest """

        return pd.concat(self.node_generator())

    @classmethod
    def from_samples(cls, samples, hierarchical_column, inclusive_aggregators, use_spark=False):
        """ Constructor meant to be called by the user """

        df_samples = util.samples_to_dataframe(samples, use_spark)

        tree_meta_columns = TreeMetaColumns(df_samples.columns, hierarchical_column, inclusive_aggregators)
        return cls(df_samples, tree_meta_columns)


class Node(pd.Series, TreeMetaColumns):

    def __init__(self, row_in_forest, forest):
        pd.Series.__init__(self, row_in_forest)
        self.copy_tree_meta_columns(forest)
        self.forest = forest

    def __repr__(self):
        return util.pretty_json_dumps(json.loads(self.to_json()))

    @property
    def children(self):
        return self.forest.children_of(self)

    def path(self, child_selector):
        return self.forest.query_path(self, child_selector)

    def hot_path(self, column):
        return self.path(lambda children: max(children, key=lambda c: c[column]))


class Tree(TreeMetaColumns):

    def __init__(self, root):
        self.copy_tree_meta_columns(root.forest)
        self.root = root

    def __repr__(self):
        return util.pretty_json_dumps(self.to_json())

    def to_json(self):
        return self.root.forest.to_json_tree(self.root)

    def hot_path(self, column):
        return self.root.hot_path(column)
