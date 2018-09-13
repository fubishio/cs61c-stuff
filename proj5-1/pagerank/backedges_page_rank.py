from simple_page_rank import SimplePageRank

"""
This class implements the pagerank algorithm with
backwards edges as described in the second part of 
the project.
"""
class BackedgesPageRank(SimplePageRank):

    """
    The implementation of __init__ and compute_pagerank should 
    still be the same as SimplePageRank.
    You are free to override them if you so desire, but the signatures
    must remain the same.
    """

    """
    This time you will be responsible for implementing the initialization
    as well. 
    Think about what additional information your data structure needs 
    compared to the old case to compute weight transfers from pressing
    the 'back' button.
    """
    @staticmethod
    def initialize_nodes(input_rdd):
        # Although the stuff inherits from SimplePageRank, I had issues since I needed to add in one more 
        # variable or parameter so that the methods match the solution pattern of (node, (weight, targets, old_weight))
        # it seems that although the stuff was inherited from simplepagerank, i had to rewrite everything


         
        # The pattern that this solution uses is to keep track of 
        # (node, (weight, targets, old_weight)) for each iteration.
        # When calculating the score for the next iteration, you
        # know that 10% of the score you sent out from the previous
        # iteration will get sent back.
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
            return (source, (1.0, targets, 1.0))                    #only change i made to the simple page rank constructors
                                                                    #i thought for some reason that these methods would be inherited from pagerank but since the only 
                                                                    #function inherited was the initialized node constructor wise, i just had to copy and edit the one from simple
        nodes = input_rdd\
                .flatMap(emit_edges)\
                .reduceByKey(reduce_edges)\
                .map(initialize_weights)
        return nodes

    """
    You will also implement update_weights and format_output from scratch.
    You may find the distribute and collect pattern from SimplePageRank
    to be suitable, but you are free to do whatever you want as long
    as it results in the correct output.
    """
    @staticmethod
    def update_weights(nodes, num_nodes):
        
        def distribute_weights((node, (weight, targets, old_weight))):
            ret = []                                    #copied most from my simple pagerank but all i changed was how the first call initialized
            returnedandstay = weight * 0.05 + old_weight*0.1           #cant just add .1 flat out, only when it returns       
            ret.append((node, returnedandstay)) 
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

        def collect_weights((node, values)):
            tempweight = 0.0
            retlist = []                                #same for simplepagerank but add tempweight since nodes have 3 elements
            for val in values:
                # if type(val == float):                #i forgot python didnt accept this syntax
                if type(val) is float:                  #had to go to office hours to get this
                    tempweight = tempweight + val
                else:
                    retlist.append(val)
            return (node, (tempweight, frozenset(retlist), tempweight))

        return nodes\
                .flatMap(distribute_weights)\
                .groupByKey()\
                .map(collect_weights)

                     
    @staticmethod
    def format_output(nodes):       #only change from simplepage was the oldweight added at the end to read and format properly
        return nodes\
                .map(lambda (node, (weight, targets, old_weight)): (weight, node))\
                .sortByKey(ascending = False)\
                .map(lambda (weight, node): (node, weight))