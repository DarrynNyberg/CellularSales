<?php
	$nid = arg(2);
	$node = node_load($nid);
	
?>

<div class="row">
	<div class="col-sm-12">
		<h1><?php print $node->title; ?></h1>
		<ul>
			<li>NID: <?php print $node->nid; ?></li>
			<li><?php print l('View', 'node/' . $node->nid); ?></li>
			<li><?php print l('Edit', 'node/' . $node->nid . '/edit', array('query' => array('destination' => current_path()))); ?></li>
		</ul>
		<pre><code>
			<?php print_r($node); ?>
		</code></pre>
	</div>
</div>
