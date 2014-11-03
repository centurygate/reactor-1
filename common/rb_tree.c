/**
# -*- coding:UTF-8 -*-
*/
#include "rb_tree.h"
#include <stdio.h>
#include <malloc.h>

rb_tree_node_t* rb_insert_rebalance(rb_tree_t *tree, rb_tree_node_t *node);
rb_tree_node_t* rb_erase_rebalance(rb_tree_t *tree, rb_tree_node_t *node, rb_tree_node_t *parent);

rb_tree_node_t* bst_left_rotate(rb_tree_t *tree, rb_tree_node_t *pivot);
rb_tree_node_t* bst_right_rotate(rb_tree_t *tree, rb_tree_node_t *pivot);


rb_tree_node_t* rb_tree_node_init(rb_tree_node_color_t color);


void rb_tree_iterator_next(rb_tree_iterator_t *p_iter)
{
	rb_tree_node_t *node = (rb_tree_node_t *)p_iter->data;

	if (node == NULL){
		return;
	}

	if (node->right != NULL){
		node = node->right;
		while (node&&node->left){
			node = node->left;
		}
		p_iter->data = node;
	}
	else{
		while (node->parent != NULL && node->parent->right == node){
			node = node->parent;
		}
		p_iter->data = node->parent;
	}
}

rb_tree_node_t* rb_tree_node_init(rb_tree_node_color_t color)
{
	rb_tree_node_t *p_node = (rb_tree_node_t *)malloc(sizeof(rb_tree_node_t));
	p_node->color = color;
	p_node->left = NULL;
	p_node->right = NULL;
	p_node->parent = NULL;
	p_node->data = NULL;

	return p_node;
}

void* rb_tree_iterator_value(rb_tree_iterator_t iter)
{
	return ((rb_tree_node_t *)iter.data)->data;
}

int32_t rb_tree_iterator_equal(rb_tree_iterator_t iter1, rb_tree_iterator_t iter2)
{
	return iter1.data == iter2.data;
}

void rb_tree_init(rb_tree_t *tree, element_type_t elem_type, associative_container_cmp_func cmp_func)
{
	tree->cmp_fun = cmp_func;
	tree->_size = 0;
	tree->root = NULL;
	tree->_key_t = elem_type;
}

rb_tree_iterator_t rb_tree_begin(rb_tree_t *p_tree)
{
	rb_tree_node_t *node = p_tree->root;
	rb_tree_iterator_t iter;

	while (node != NULL && node->left != NULL){
		node = node->left;
	}

	iter.data = node;

	return iter;
}

rb_tree_iterator_t rb_tree_end(rb_tree_t *p_tree)
{
	rb_tree_iterator_t iter = { NULL };
	return iter;
}

void rb_tree_erase_iterator(rb_tree_t *p_tree, rb_tree_iterator_t iter)
{
	rb_tree_erase_node(p_tree, (rb_tree_node_t *)iter.data);
}

static rb_tree_node_t* rb_tree_find_impl(rb_tree_t* tree, const void *data, rb_tree_node_t **pp_parent)
{
	rb_tree_node_t *node = tree->root, *parent = NULL;
	int64_t cmp_ret;

	while (node != NULL){
		cmp_ret = (*tree->cmp_fun)(tree, data, node->data);
		parent = node;

		if (cmp_ret == 0){
			return node;
		}
		else if (cmp_ret < 0){
			node = node->left;
		}
		else{
			node = node->right;
		}
	}

	if (pp_parent != NULL){
		*pp_parent = parent;
	}

	return node;
}

rb_tree_node_t* rb_tree_find(rb_tree_t* tree, const void *data)
{
	return rb_tree_find_impl(tree, data, NULL);
}

/**
 * return: the node just inserted
 *
 */
rb_tree_node_t* rb_tree_insert(rb_tree_t* tree, void *data)
{
	rb_tree_node_t *parent = NULL, *node;

	if ((node = rb_tree_find_impl(tree, data, &parent)) != NULL){
		return NULL;
	}

	node = (rb_tree_node_t *)malloc(sizeof(rb_tree_node_t));
	node->parent = parent;
	node->left = node->right = NULL;
	node->color = RED;
	node->data = data;

	if (parent != NULL)
	{
		int32_t cmp_ret = (*tree->cmp_fun)(tree, data, parent->data);
		if (cmp_ret < 0)
		{
			parent->left = node;
		}
		else
		{
			parent->right = node;
		}
	}
	else
	{
		tree->root = node;
	}

	++tree->_size;
	return rb_insert_rebalance(tree, node);
}


rb_tree_node_t* rb_insert_rebalance(rb_tree_t *tree, rb_tree_node_t *node)
{
	rb_tree_node_t *parent, *gparent, *uncle, *tmp;

	while ((parent = node->parent) && parent->color == RED)
	{
		gparent = parent->parent;

		if (parent == gparent->left)
		{
			uncle = gparent->right;

			if (uncle && uncle->color == RED)
			{
				uncle->color = BLACK;
				parent->color = BLACK;
				gparent->color = RED;
				node = gparent;
			}
			else
			{
				if (parent->right == node)
				{
					tree->root = bst_left_rotate(tree, parent);
					tmp = parent;
					parent = node;
					node = tmp;
				}

				parent->color = BLACK;
				gparent->color = RED;
				tree->root = bst_right_rotate(tree, gparent);
			}
		}

		else
		{

			uncle = gparent->left;
			if (uncle && uncle->color == RED)
			{
				uncle->color = BLACK;
				parent->color = BLACK;
				gparent->color = RED;
				node = gparent;
			}
			else
			{
				if (parent->left == node)
				{
					tree->root = bst_right_rotate(tree, parent);
					tmp = parent;
					parent = node;
					node = tmp;
				}

				parent->color = BLACK;
				gparent->color = RED;
				tree->root = bst_left_rotate(tree, gparent);
			}
		}
	}

	tree->root->color = BLACK;
	return tree->root;
}

rb_tree_node_t* rb_erase_rebalance(rb_tree_t *tree, rb_tree_node_t *node, rb_tree_node_t *parent)
{
	rb_tree_node_t *other, *o_left, *o_right;

	while ((!node || node->color == BLACK) && node != tree->root)
	{
		if (parent->left == node)
		{
			other = parent->right;
			if (other->color == RED)
			{
				other->color = BLACK;
				parent->color = RED;
				tree->root = bst_left_rotate(tree, parent);
				other = parent->right;
			}
			if ((!other->left || other->left->color == BLACK) &&
				(!other->right || other->right->color == BLACK))
			{
				other->color = RED;
				node = parent;
				parent = node->parent;
			}
			else
			{
				if (!other->right || other->right->color == BLACK)
				{
					if ((o_left = other->left))
					{
						o_left->color = BLACK;
					}
					other->color = RED;
					bst_right_rotate(tree, other);
					other = parent->right;
				}


				other->color = parent->color;
				parent->color = BLACK;
				if (other->right)
				{
					other->right->color = BLACK;
				}
				bst_left_rotate(tree, parent);
				node = tree->root;
				break;
			}
		}

		else
		{
			other = parent->left;
			if (other->color == RED)
			{
				other->color = BLACK;
				parent->color = RED;
				bst_right_rotate(tree, parent);
				other = parent->left;
			}
			if ((!other->left || other->left->color == BLACK) &&
				(!other->right || other->right->color == BLACK))
			{
				other->color = RED;
				node = parent;
				parent = node->parent;
			}
			else
			{
				if (!other->left || other->left->color == BLACK)
				{
					if ((o_right = other->right))
					{
						o_right->color = BLACK;
					}
					other->color = RED;
					tree->root = bst_left_rotate(tree, other);
					other = parent->left;
				}
				other->color = parent->color;
				parent->color = BLACK;
				if (other->left)
				{
					other->left->color = BLACK;
				}
				bst_right_rotate(tree, parent);
				node = tree->root;
				break;
			}
		}
	}

	if (node)
	{
		node->color = BLACK;
	}
	return tree->root;
}


void rb_tree_erase_node(rb_tree_t *tree, rb_tree_node_t *node)
{
	rb_tree_node_t *child, *parent, *old, *left;
	rb_tree_node_color_t color;


	old = node;

	if (node->left && node->right)
	{
		node = node->right;
		while ((left = node->left) != NULL)
		{
			node = left;
		}
		child = node->right;
		parent = node->parent;
		color = node->color;

		if (child)
		{
			child->parent = parent;
		}
		if (parent)
		{
			if (parent->left == node)
			{
				parent->left = child;
			}
			else
			{
				parent->right = child;
			}
		}
		else
		{
			tree->root = child;
		}

		if (node->parent == old)
		{
			parent = node;
		}

		node->parent = old->parent;
		node->color = old->color;
		node->right = old->right;
		node->left = old->left;

		if (old->parent)
		{
			if (old->parent->left == old)
			{
				old->parent->left = node;
			}
			else
			{
				old->parent->right = node;
			}
		}
		else
		{
			tree->root = node;
		}

		old->left->parent = node;
		if (old->right)
		{
			old->right->parent = node;
		}
	}
	else
	{
		if (!node->left)
		{
			child = node->right;
		}
		else if (!node->right)
		{
			child = node->left;
		}
		parent = node->parent;
		color = node->color;

		if (child) ///TODO:init
		{
			child->parent = parent;
		}
		if (parent)
		{
			if (parent->left == node)
			{
				parent->left = child;
			}
			else
			{
				parent->right = child;
			}
		}
		else
		{
			tree->root = child;
		}
	}

	free(old);
	--tree->_size;

	if (color == BLACK)
	{
		tree->root = rb_erase_rebalance(tree, child, parent);
	}
}

void rb_tree_erase(rb_tree_t *tree, const void *data)
{
	rb_tree_node_t *node = rb_tree_find(tree, data);
	if (node != NULL)
	{
		rb_tree_erase_node(tree, node);
	}
}

void rb_recursive_delete(rb_tree_node_t *p_node)
{
	if (NULL == p_node)
	{
		return;
	}

	if (p_node->right != NULL) rb_recursive_delete(p_node->right);
	if (p_node->left != NULL) rb_recursive_delete(p_node->left);

	free(p_node->data);
	free(p_node);
}

rb_tree_node_t* bst_left_rotate(rb_tree_t *tree, rb_tree_node_t *node)
{
	rb_tree_node_t* right = node->right;

	if ((node->right = right->left) != NULL)
	{
		right->left->parent = node;
	}
	right->left = node;

	if ((right->parent = node->parent) != NULL)
	{
		if (node == node->parent->right)
		{
			node->parent->right = right;
		}
		else
		{
			node->parent->left = right;
		}
	}
	else
	{
		tree->root = right;
	}
	node->parent = right;

	return tree->root;
}

rb_tree_node_t* bst_right_rotate(rb_tree_t *tree, rb_tree_node_t *node)
{
	rb_tree_node_t* left = node->left;

	if ((node->left = left->right) != NULL)
	{
		left->right->parent = node;
	}
	left->right = node;

	if ((left->parent = node->parent) != NULL)
	{
		if (node == node->parent->right)
		{
			node->parent->right = left;
		}
		else
		{
			node->parent->left = left;
		}
	}
	else
	{
		tree->root = left;
	}
	node->parent = left;

	return tree->root;
}
