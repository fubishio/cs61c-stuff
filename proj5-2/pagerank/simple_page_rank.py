"""
This class implements the simple pagerank algorithm
as described in the first part of the project.
"""
class SimplePageRank(object):

    """
    Keeps track of the rdd used as the input data. 
    This should be a list of lines similar to the example input files.
    You do not need to change this method, but feel free to do so to suit your needs. 
    However, the signature MUST stay the same.
    """
    def __init__(self, input_rdd):
        self.input_rdd = input_rdd

    """
    Computes the pagerank algorithm for num_iters number of iterations.
    You do not need to change this method, but feel free to do so to suit your needs. 
    However, the signature MUST stay the same.
    The output should be a rdd of (pagerank score, node label) pairs, 
    sorted by pagerank score in descending order.
    """
    def compute_pagerank(self, num_iters):
        nodes = self.initialize_nodes(self.input_rdd)
        num_nodes = nodes.count()
        for i in range(0, num_iters):
            nodes = self.update_weights(nodes, num_nodes)
        return self.format_output(nodes)

    """
    Converts the input_rdd to a rdd suitable for iteration with
    the pagerank update algorithm.
    The rdd nodes should be enough to calculate the next iteration
    of the pagerank update algorithm by itself, without using any
    external structures.
    That means that all the edges must be stored somewhere,
    as well as the current weights of each node.
    You do not need to change this method, but feel free to do so to suit your needs.
    In the default implemention, the rdd is simply a collection of (node label, (current weight, target)) tuples.
    Lines in the input_rdd file will either be blank or begin with "#", which
    should be ignored, or be of the form "source[whitespace]target" where source and target
    are labels for nodes that are integers. 
    For example, the line:
    1    3
    tells us that there is an edge going from node 1 to node 3.
    """
    @staticmethod
    def initialize_nodes(input_rdd):
        # takes in a line and emits edges in the graph corresponding to that line
        def emit_edges(line):
            # ignore blank lines and comments
            if len(line) == 0 or line[0] == "#":
                return []
            # get the source and target labels
            source, target = tuple(map(int, line.split()))
            # emit the edge
            edge = (source, frozenset([target]))
            # also emit "empty" edges to catch nodes that do not have any
            # other node leading into them, but we still want in our list of nodes
            self_source = (source, frozenset())
            self_target = (target, frozenset())
            return [edge, self_source, self_target]

        # collects all outgoing target nodes for a given source node
        def reduce_edges(e1, e2):
            return e1 | e2 

        # sets the weight of every node to 0, and formats the output to the 
        # specified format of (source (weight, targets))
        def initialize_weights((source, targets)):
            return (source, (1.0, targets))

        nodes = input_rdd\
                .flatMap(emit_edges)\
                .reduceByKey(reduce_edges)\
                .map(initialize_weights)
        return nodes

    """
    Performs one iteration of the pagerank update algorithm on the set of node data.
    Details about the update algorithm are in the spec on the website.
    You are allowed to change the signature if you desire to.
    """
    @staticmethod
    def update_weights(nodes, num_nodes):
        """
        Mapper phase.
        Distributes pagerank scores for a given node to each of its targets,
        as specified by the update algorithm.
        Some important things to consider:
        We can't just emit (target, weight) values to the reduce phase, 
        because then the reduce phase will lose information on the outgoing edges
        for the nodes. We have to emit the (node, targets) pairs too so that 
        the edges can be remembered for the next iteration.
        Think about the best output format for the mapper so the reducer can
        get both types of information.
        You are allowed to change the signature if you desire to.
        """
        def distribute_weights((node, (weight, targets))):
            ret = []                                    #initilize, its the same for all
            staynrandom = weight * 0.05 + 0.1           #had to add .1 here, i was bugging out because i forgot the random case            
            ret.append((node, staynrandom)) 
            # if (targets != None):
            if (targets):                                 #forgot that python accepted this syntax too and that nonzero is a true
                for target in targets:                  #case where there are actual links
                    ret.append((node, target))
                    ret.append((target, (weight * 0.85 / len(targets))))

            else:                                       #no links so rest was designated to random
                for tempnode in range(num_nodes):
                    if tempnode != node:
                        ret.append((tempnode, (weight * 0.85 / (num_nodes - 1))))
            return ret

            
            # temp1 = []
            # for j in weight:
            #     temp1.append(j)
                # counter = counter + 1
            # if len(line) == 0 or line[0] == "#":
            #     return []
            # # get the source and target labels
            # source, target = tuple(map(int, line.split()))
            # # emit the edge
            # edge = (source, frozenset([target]))
            # # also emit "empty" edges to catch nodes that do not have any
            # # other node leading into them, but we still want in our list of nodes
            # self_source = (source, frozenset())
            # self_target = (target, frozenset())
            # return [edge, self_source, self_target]
            # return node, temp1, temp, lenoftarg            #reference to return type of constructors

            # nodemap = {}
            # weightmap = {}
            # targetmap = {}
            # print(nodemap)
            # for i in range(num_nodes):
            #     nodemap[i] = node
            #     targetmap[i] = targets
            #     weightmap[i] = .05 + .1
            # for j in range(num_nodes):
            #     counter = 0
            #     for k in range(targetmap[j]):
            #         counter = counter + 0.85/len(targetmap[k])
                    


            #     weightmap[j] = weightmap[j] + counter
            # ret = {}
            # for l in range(num_nodes):
            #     ret[nodemap[l]] = tuple(weightmap[l], targetmap[l])
            # return ret
        """
        Reducer phase.
        We are given a node as a key and a list of all the values emitted by the mappers
        corresponding to that key.
        There should be two types of values:
        Pagerank scores, which represent how much score an incoming node is giving to us,
        and edge data, which we need to collect and store for the next iteration.
        The output of this phase should be in the same format as the input to the mapper.
        You are allowed to change the signature if you desire to.
        """
        def collect_weights((node, values)):
            tempweight = 0.0
            retlist = []
            for val in values:
                # if type(val == float):                #i forgot python didnt accept this syntax
                if type(val) is float:                  #had to go to office hours to get this, totally
                    tempweight = tempweight + val
                else:
                    retlist.append(val)
            return (node, (tempweight, frozenset(retlist)))

        return nodes\
                .flatMap(distribute_weights)\
                .groupByKey()\
                .map(collect_weights)

            # collect_weights(0)
            # print(len(targets))
            # lenoftarg = len(targets)
            # counter = 0
            # temp = []
            # for i in targets:
            #     temp.append(i)
            # valmap = {}
            # counter = 0
            # for i in values
            # valmap[counter] = i
            # counter = counter + 1
            # return valmap


    """
    Formats the output of the data to the format required by the specs.
    If you changed the format of the update_weights method you will 
    have to change this as well.
    Otherwise, this is fine as is.
    """
    @staticmethod
    def format_output(nodes):
        return nodes\
                .map(lambda (node, (weight, targets)): (weight, node))\
                .sortByKey(ascending = False)\
                .map(lambda (weight, node): (node, weight))
