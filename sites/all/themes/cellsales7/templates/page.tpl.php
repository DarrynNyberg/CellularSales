

<header id="header-top">
	<div class="container">
		<div class="col-xs-3">
			<a class="logo " href="<?php print $front_page; ?>" title="<?php print t('Home'); ?>">
	        	<img class="hidden-md hidden-sm hidden-xs" src="<?php print $logo; ?>" alt="<?php print t('Home'); ?>" />
	        	<img class="visible-md visible-sm visible-xs logo-sm" src="<?php print VTHEME; ?>img/logo_small.png" alt="<?php print t('Home'); ?>" />
	    	</a>
		</div>
		<div class="col-xs-9">
			<div class="hidden-xs">
				<?php print render($page['navigation']); ?>
			</div>
			<div class="visible-xs">
				<a href="#" class="toggle-phone-menu pull-right toggle-menu toggle-link">
					<span class="glyphicon glyphicon-align-justify" ></span>
				</a>
			</div>
		</div>
	</div>
</header>
<header id="header-middle">
	<div class="container">
		<div class="col-sm-12">
			 <?php print render($page['header']); ?>
		</div>
	</div>
</header>
<header id="header-bottom">
	<div class="container">
		<div class="col-sm-12">
			<h1 class="page-header"><?php print $title; ?></h1>
		</div>
	</div>
</header>

<div class="main-container">
<div class="container">
  <div class="row">
	<div class="col-sm-12">
		<div class="row">
		    <?php if (!empty($page['sidebar_first'])): ?>
              <aside class="col-sm-3" role="complementary">
                <?php print render($page['sidebar_first']); ?>
              </aside>  <!-- /#sidebar-first -->
            <?php endif; ?>
        
            <section<?php print $content_column_class; ?>>
              <?php if (!empty($page['highlighted'])): ?>
                <div class="highlighted jumbotron"><?php print render($page['highlighted']); ?></div>
              <?php endif; ?>
              <?php if (!empty($breadcrumb)): print $breadcrumb; endif;?>
              <a id="main-content"></a>
              
              <?php if($is_admin): ?>
                <?php print $messages; ?>
              <?php endif; ?>
              <?php if (!empty($tabs)): ?>
                <?php print render($tabs); ?>
              <?php endif; ?>
              <?php if (!empty($page['help'])): ?>
                <?php print render($page['help']); ?>
              <?php endif; ?>
              <?php if (!empty($action_links)): ?>
                <ul class="action-links"><?php print render($action_links); ?></ul>
              <?php endif; ?>
              <?php print render($page['content']); ?>
            </section>
        
            <?php if (!empty($page['sidebar_second'])): ?>
              <aside class="col-sm-3" role="complementary">
                <?php print render($page['sidebar_second']); ?>
              </aside>  <!-- /#sidebar-second -->
            <?php endif; ?>
		</div>
	</div>
  </div>
</div>
</div>
<footer class="footer">
	<div class="container">
	  <div class="col-sm-12">
	  	<?php print render($page['footer']); ?>
	  </div>
	</div>
</footer>


<div class="menu-block out">
	<div class="region region-navigation">
		<section id="phone-menu" class="block block-block clearfix">
			<ul class="nav">
				<li class="menu-close">
					<a href="#" class="toggle-menu toggle-link">
						<span class="glyphicon glyphicon-arrow-right pull-right" aria-hidden="true"></span>
						<div class="clearfix"></div>
					</a>
				</li>				
			</ul>
			<?php
				$phone_menu = menu_navigation_links('menu-top-menu-2');
				print theme('links__menu_top_menu-2', array(
					'links' => $phone_menu,
					'attributes' => array(
						'class' => 'nav',
					),
				));
			?>
		</section> <!-- /.block -->
	</div>
</div>