 <script>
    // You can also use "$(window).load(function() {"
    jQuery(function () {

      // Slideshow 1
      jQuery("#slider1").responsiveSlides({
			maxwidth: 800,
			speed: 800,
			nav: true,
			pauseControls: true,			
			prevText: "Previous",
			nextText: "Next",  		
		});	  
    });
  </script>
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

<div class="main-container container">
  <div class="row">
	<div class="col-sm-12">
		   
	</div>
  </div>
  <div class="row home-row-header">

    <?php if (!empty($page['sidebar_first'])): ?>
      <aside class="col-sm-3" role="complementary">
        <?php print render($page['sidebar_first']); ?>
      </aside>  <!-- /#sidebar-first -->
    <?php endif; ?>

    <section id="home-col-9" <?php print $content_column_class; ?>>
      <?php print render($tabs); ?>
      <?php print $messages; ?>
      <?php print render($page['content']); ?>
    </section>

    <?php if (!empty($page['sidebar_second'])): ?>
      <aside class="col-sm-3" role="complementary">
        <?php print render($page['sidebar_second']); ?>
      </aside>  <!-- /#sidebar-second -->
    <?php endif; ?>

  </div>
</div>
<footer class="footer">
	<div class="container">
		<div class="col-sm-12">
			<?php print render($page['footer']); ?>
		</div>
	</div>
</footer>
<footer class="footer-bottom">
	<div class="container">
		<div class="col-sm-12">
			<?php print render($page['footer_bottom']); ?>
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